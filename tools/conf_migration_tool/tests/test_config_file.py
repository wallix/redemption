#!/usr/bin/env python3
import hashlib
import os
import unittest

from typing import List
from conf_migrate import (ConfigurationFile,
                          ConfigurationLine,
                          read_configuration_lines,
                          RedemptionVersion)

class Test_ConfigurationFile(unittest.TestCase):
    def test_load_save(self):
        target_file = './rdpproxy.sav'

        try:
            os.remove(target_file)
        except:
            pass

        config_lines = read_configuration_lines('./tests/fixtures/rdpproxy.ini')
        configuration_file = ConfigurationFile(config_lines)

        configuration_file.save_to(target_file)

        with open(target_file, 'rb') as f:
            self.assertEqual(hashlib.md5(f.read()).hexdigest(),
                             'ffb503380ff480ced4c26ba4ce3b42bb')

        os.remove(target_file)

def test_migrate_line_to_9_0_0(section_name, line):
    if line.is_variable_declaration():
        if "section" == section_name:
            if line.get_name() == "value_8_2_0":
                return None, f"old_value = {line.get_value()}"

            elif line.get_name() == "value_2_8_2_0":
                return None, f"old_value_2 = {line.get_value()}"

def test_migrate_line_to_9_1_0(section_name, line):
    if line.is_variable_declaration():
        if "section" == section_name:
            if line.get_name() == "old_value":
                return None, f"new_value = {line.get_value()}"

            elif line.get_name() == "old_value_2":
                return "new_section", f"new_value_2 = {line.get_value()}"

migration_funcs = (
    (RedemptionVersion("9.0.0"), test_migrate_line_to_9_0_0),
    (RedemptionVersion("9.1.0"), test_migrate_line_to_9_1_0),
)

class Test_RedemptionConfigurationFile(unittest.TestCase):
    maxDiff = None

    def assertConfLinesEqual(self,
                             lines1:List[ConfigurationLine],
                             lines2:List[ConfigurationLine]) -> None:
        s1 = '\n'.join(map(str, lines1))
        s2 = '\n'.join(map(str, lines2))
        self.assertEqual(f'{s1}\nlen: {len(lines1)}',
                         f'{s2}\nlen: {len(lines2)}')

    def test_migrate_to_9_1_0_empty(self):
        verbose = False

        configuration_file = ConfigurationFile([], verbose)

        configuration_file.migrate(migration_funcs, RedemptionVersion("9.0.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_empty: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = []

        self.assertConfLinesEqual(expected_content, configuration_file._content)

    def test_migrate_to_9_1_0_section_old_value(self):
        verbose = False

        configuration_file = ConfigurationFile([], verbose)

        configuration_file._content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("old_value = 1234", verbose)
        ]

        configuration_file.migrate(migration_funcs, RedemptionVersion("9.0.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_section_old_value: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("#old_value = 1234", verbose),
            ConfigurationLine("new_value = 1234", verbose)
        ]

        self.assertConfLinesEqual(expected_content, configuration_file._content)

    def test_migrate_to_9_1_0_section_old_value_b(self):
        verbose = False

        configuration_file = ConfigurationFile([], verbose)

        configuration_file._content = [
            ConfigurationLine("", verbose),
            ConfigurationLine("", verbose),
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("old_value = 1234", verbose)
        ]

        configuration_file.migrate(migration_funcs, RedemptionVersion("9.0.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_section_old_value_b: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationLine("", verbose),
            ConfigurationLine("", verbose),
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("#old_value = 1234", verbose),
            ConfigurationLine("new_value = 1234", verbose)
        ]

        self.assertConfLinesEqual(expected_content, configuration_file._content)

    def test_migrate_to_9_1_0_section_old_value_c(self):
        verbose = False

        configuration_file = ConfigurationFile([], verbose)

        configuration_file._content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("old_value = 1234", verbose),
            ConfigurationLine("", verbose),
            ConfigurationLine("", verbose)
        ]

        configuration_file.migrate(migration_funcs, RedemptionVersion("9.0.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_section_old_value_c: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("#old_value = 1234", verbose),
            ConfigurationLine("new_value = 1234", verbose),
            ConfigurationLine("", verbose),
            ConfigurationLine("", verbose)
        ]

        self.assertConfLinesEqual(expected_content, configuration_file._content)

    def test_migrate_to_9_1_0_section_old_value_d(self):
        verbose = False

        configuration_file = ConfigurationFile([], verbose)

        configuration_file._content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("", verbose),
            ConfigurationLine("old_value = 1234", verbose),
            ConfigurationLine("", verbose)
        ]

        configuration_file.migrate(migration_funcs, RedemptionVersion("9.0.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_section_old_value_d: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("", verbose),
            ConfigurationLine("#old_value = 1234", verbose),
            ConfigurationLine("new_value = 1234", verbose),
            ConfigurationLine("", verbose)
        ]

        self.assertConfLinesEqual(expected_content, configuration_file._content)

    def test_migrate_to_9_1_0_section_old_value_2(self):
        verbose = False

        configuration_file = ConfigurationFile([], verbose)

        configuration_file._content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("old_value_2 = 1234", verbose)
        ]

        configuration_file.migrate(migration_funcs, RedemptionVersion("9.0.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_section_old_value_2: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("#old_value_2 = 1234", verbose),
            ConfigurationLine("", verbose),
            ConfigurationLine("[new_section]", verbose),
            ConfigurationLine("new_value_2 = 1234", verbose)
        ]

        self.assertConfLinesEqual(expected_content, configuration_file._content)

    def test_migrate_to_9_1_0_section_old_value_2_b(self):
        verbose = False

        configuration_file = ConfigurationFile([], verbose)

        configuration_file._content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("old_value_2 = 1234", verbose),
            ConfigurationLine("[new_section]", verbose),
        ]

        configuration_file.migrate(migration_funcs, RedemptionVersion("9.0.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_section_old_value_2_b: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("#old_value_2 = 1234", verbose),
            ConfigurationLine("[new_section]", verbose),
            ConfigurationLine("new_value_2 = 1234", verbose)
        ]

        self.assertConfLinesEqual(expected_content, configuration_file._content)

    def test_migrate_to_9_1_0_section_old_value_2_c(self):
        verbose = False

        configuration_file = ConfigurationFile([], verbose)

        configuration_file._content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("old_value_2 = 1234", verbose),
            ConfigurationLine("[new_section]", verbose),
            ConfigurationLine("", verbose)
        ]

        configuration_file.migrate(migration_funcs, RedemptionVersion("9.0.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_section_old_value_2_c: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("#old_value_2 = 1234", verbose),
            ConfigurationLine("[new_section]", verbose),
            ConfigurationLine("new_value_2 = 1234", verbose),
            ConfigurationLine("", verbose)
        ]

        self.assertConfLinesEqual(expected_content, configuration_file._content)

    def test_migrate_to_9_1_0_section_old_value_2_d(self):
        verbose = False

        configuration_file = ConfigurationFile([], verbose)

        configuration_file._content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("old_value_2 = 1234", verbose),
            ConfigurationLine("[new_section]", verbose),
            ConfigurationLine("", verbose),
            ConfigurationLine("", verbose),
            ConfigurationLine("", verbose)
        ]

        configuration_file.migrate(migration_funcs, RedemptionVersion("9.0.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_section_old_value_2_d: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("#old_value_2 = 1234", verbose),
            ConfigurationLine("[new_section]", verbose),
            ConfigurationLine("new_value_2 = 1234", verbose),
            ConfigurationLine("", verbose),
            ConfigurationLine("", verbose),
            ConfigurationLine("", verbose)
        ]

        self.assertConfLinesEqual(expected_content, configuration_file._content)

    def test_migrate_to_9_1_0_section_old_value_2_e(self):
        verbose = False

        configuration_file = ConfigurationFile([], verbose)

        configuration_file._content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("old_value_2 = 1234", verbose),
            ConfigurationLine("", verbose)
        ]

        configuration_file.migrate(migration_funcs, RedemptionVersion("9.0.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_section_old_value_2_e: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("#old_value_2 = 1234", verbose),
            ConfigurationLine("", verbose),
            ConfigurationLine("[new_section]", verbose),
            ConfigurationLine("new_value_2 = 1234", verbose)
        ]

        self.assertConfLinesEqual(expected_content, configuration_file._content)

    def test_migrate_to_9_1_0_section_old_value_2_f(self):
        verbose = False

        configuration_file = ConfigurationFile([], verbose)

        configuration_file._content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("old_value_2 = 1234", verbose),
            ConfigurationLine("", verbose),
            ConfigurationLine("", verbose)
        ]

        configuration_file.migrate(migration_funcs, RedemptionVersion("9.0.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_section_old_value_2_f: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("#old_value_2 = 1234", verbose),
            ConfigurationLine("", verbose),
            ConfigurationLine("[new_section]", verbose),
            ConfigurationLine("new_value_2 = 1234", verbose),
            ConfigurationLine("", verbose)
        ]

        self.assertConfLinesEqual(expected_content, configuration_file._content)

    def test_migrate_to_9_1_0_section_old_value_2_g(self):
        verbose = False

        configuration_file = ConfigurationFile([], verbose)

        configuration_file._content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("old_value_2 = 1234", verbose),
            ConfigurationLine("", verbose),
            ConfigurationLine("", verbose),
            ConfigurationLine("[new_section]", verbose),
            ConfigurationLine("", verbose)
        ]

        configuration_file.migrate(migration_funcs, RedemptionVersion("9.0.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_section_old_value_2_g: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("#old_value_2 = 1234", verbose),
            ConfigurationLine("", verbose),
            ConfigurationLine("", verbose),
            ConfigurationLine("[new_section]", verbose),
            ConfigurationLine("new_value_2 = 1234", verbose),
            ConfigurationLine("", verbose)
        ]

        self.assertConfLinesEqual(expected_content, configuration_file._content)

    def test_migrate_to_9_1_0_section_old_value_2_h(self):
        verbose = False

        configuration_file = ConfigurationFile([], verbose)

        configuration_file._content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("old_value_2 = 1234", verbose),
            ConfigurationLine("[new_section]", verbose),
            ConfigurationLine("any_value = 1234", verbose),
            ConfigurationLine("[other_section]", verbose),
        ]

        configuration_file.migrate(migration_funcs, RedemptionVersion("9.0.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_section_old_value_2_h: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("#old_value_2 = 1234", verbose),
            ConfigurationLine("[new_section]", verbose),
            ConfigurationLine("any_value = 1234", verbose),
            ConfigurationLine("", verbose),
            ConfigurationLine("new_value_2 = 1234", verbose),
            ConfigurationLine("[other_section]", verbose),
        ]

        self.assertConfLinesEqual(expected_content, configuration_file._content)

    def test_migrate_to_9_1_0_from_8_2_0(self):
        verbose = False

        configuration_file = ConfigurationFile([], verbose)

        configuration_file._content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("value_8_2_0 = 1234", verbose),
            ConfigurationLine("", verbose)
        ]

        configuration_file.migrate(migration_funcs, RedemptionVersion("8.2.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_from_8_2_0: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("#value_8_2_0 = 1234", verbose),
            ConfigurationLine("new_value = 1234", verbose),
            ConfigurationLine("", verbose)
        ]

        self.assertConfLinesEqual(expected_content, configuration_file._content)

    def test_migrate_to_9_1_0_from_8_2_0_b(self):
        verbose = False

        configuration_file = ConfigurationFile([], verbose)

        configuration_file._content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("value_2_8_2_0 = 1234", verbose),
            ConfigurationLine("", verbose)
        ]

        configuration_file.migrate(migration_funcs, RedemptionVersion("8.2.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_from_8_2_0_b: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("#value_2_8_2_0 = 1234", verbose),
            ConfigurationLine("", verbose),
            ConfigurationLine("[new_section]", verbose),
            ConfigurationLine("new_value_2 = 1234", verbose)
        ]

        self.assertConfLinesEqual(expected_content, configuration_file._content)

    def test_migrate_to_9_1_0_from_8_2_0_c(self):
        verbose = False

        configuration_file = ConfigurationFile([], verbose)

        configuration_file._content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("other_value = 1234", verbose),
            ConfigurationLine("", verbose),
            ConfigurationLine("[other_section]", verbose),
            ConfigurationLine("other_value = 1234", verbose),
            ConfigurationLine("", verbose)
        ]

        configuration_file.migrate(migration_funcs, RedemptionVersion("8.2.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_from_8_2_0_c: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("other_value = 1234", verbose),
            ConfigurationLine("", verbose),
            ConfigurationLine("[other_section]", verbose),
            ConfigurationLine("other_value = 1234", verbose),
            ConfigurationLine("", verbose)
        ]

        self.assertConfLinesEqual(expected_content, configuration_file._content)

    def test_migrate_to_9_1_0_from_8_2_0_d(self):
        verbose = False

        configuration_file = ConfigurationFile([], verbose)

        configuration_file._content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("value_2_8_2_0 = 1234", verbose),
            ConfigurationLine("", verbose),
            ConfigurationLine("[other_section]", verbose),
            ConfigurationLine("other_value = 1234", verbose)
        ]

        configuration_file.migrate(migration_funcs, RedemptionVersion("8.2.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_from_8_2_0_d: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationLine("[section]", verbose),
            ConfigurationLine("#value_2_8_2_0 = 1234", verbose),
            ConfigurationLine("", verbose),
            ConfigurationLine("[other_section]", verbose),
            ConfigurationLine("other_value = 1234", verbose),
            ConfigurationLine("", verbose),
            ConfigurationLine("[new_section]", verbose),
            ConfigurationLine("new_value_2 = 1234", verbose)
        ]

        self.assertConfLinesEqual(expected_content, configuration_file._content)
