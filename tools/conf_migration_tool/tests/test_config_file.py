import hashlib
import os
import unittest

from conf_migrate import ConfigurationFile, ConfigurationFileLine,          \
    RedemptionVersion

class Test_ConfigurationFile(unittest.TestCase):
    def test_load_save(self):
        target_file = './rdpproxy.sav'

        try:
            os.remove(target_file)
        except:
            pass

        configuration_file = ConfigurationFile('./tests/fixtures/rdpproxy.ini')

        configuration_file.save_to(target_file)

        f = open(target_file, 'rb')

        self.assertEqual(hashlib.md5(f.read()).hexdigest(),
            'ffb503380ff480ced4c26ba4ce3b42bb')

        f.close()

        os.remove(target_file)

class TestRedemptionConfigurationFile(ConfigurationFile):
    def _get_line_migration_func(self, previous_version):
        noneable_line_migration_func = None
        noneable_result_version = None

        v_9_0_0 = RedemptionVersion("9.0.0")
        v_9_1_0 = RedemptionVersion("9.1.0")

        if previous_version < v_9_0_0:
            noneable_line_migration_func = self.__migrate_line_to_9_0_0
            noneable_result_version = v_9_0_0

        elif previous_version < v_9_1_0:
            noneable_line_migration_func = self.__migrate_line_to_9_1_0
            noneable_result_version = v_9_1_0

        return noneable_line_migration_func, noneable_result_version

    def __migrate_line_to_9_0_0(self, section_name, line):
        keep_unchanged = True
        noneable_dest_section_name = None
        noneable_line_raw_data = None

        if line.is_variable_declaration():
            if "section" == section_name:
                if line.get_name() == "value_8_2_0":
                    keep_unchanged = False
                    noneable_dest_section_name = None
                    noneable_line_raw_data =                                \
                        f"old_value = {line.get_value()}"

                elif line.get_name() == "value_2_8_2_0":
                    keep_unchanged = False
                    noneable_dest_section_name = None
                    noneable_line_raw_data =                                \
                        f"old_value_2 = {line.get_value()}"

        return keep_unchanged, noneable_dest_section_name,                  \
            noneable_line_raw_data

    def __migrate_line_to_9_1_0(self, section_name, line):
        keep_unchanged = True
        noneable_dest_section_name = None
        noneable_line_raw_data = None

        if line.is_variable_declaration():
            if "section" == section_name:
                if line.get_name() == "old_value":
                    keep_unchanged = False
                    noneable_dest_section_name = None
                    noneable_line_raw_data =                                \
                        f"new_value = {line.get_value()}"

                elif line.get_name() == "old_value_2":
                    keep_unchanged = False
                    noneable_dest_section_name = "new_section"
                    noneable_line_raw_data =                                \
                        f"new_value_2 = {line.get_value()}"

        return keep_unchanged, noneable_dest_section_name,                  \
            noneable_line_raw_data

class Test_RedemptionConfigurationFile(unittest.TestCase):
    def test_migrate_to_9_1_0_empty(self):
        verbose = False

        configuration_file = TestRedemptionConfigurationFile(None, verbose)

        configuration_file.migrate(RedemptionVersion("9.0.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_empty: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = []

        self.assertEqual(expected_content, configuration_file._content)
        self.assertEqual(len(expected_content),
            len(configuration_file._content))

    def test_migrate_to_9_1_0_section_old_value(self):
        verbose = False

        configuration_file = TestRedemptionConfigurationFile(None, verbose)

        configuration_file._content = [
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("old_value = 1234", verbose)
        ]

        configuration_file.migrate(RedemptionVersion("9.0.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_section_old_value: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("#old_value = 1234", verbose),
            ConfigurationFileLine("new_value = 1234", verbose)
        ]

        self.assertEqual(expected_content, configuration_file._content)
        self.assertEqual(len(expected_content),
            len(configuration_file._content))

    def test_migrate_to_9_1_0_section_old_value_b(self):
        verbose = False

        configuration_file = TestRedemptionConfigurationFile(None, verbose)

        configuration_file._content = [
            ConfigurationFileLine("", verbose),
            ConfigurationFileLine("", verbose),
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("old_value = 1234", verbose)
        ]

        configuration_file.migrate(RedemptionVersion("9.0.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_section_old_value_b: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationFileLine("", verbose),
            ConfigurationFileLine("", verbose),
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("#old_value = 1234", verbose),
            ConfigurationFileLine("new_value = 1234", verbose)
        ]

        self.assertEqual(expected_content, configuration_file._content)
        self.assertEqual(len(expected_content),
            len(configuration_file._content))

    def test_migrate_to_9_1_0_section_old_value_c(self):
        verbose = False

        configuration_file = TestRedemptionConfigurationFile(None, verbose)

        configuration_file._content = [
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("old_value = 1234", verbose),
            ConfigurationFileLine("", verbose),
            ConfigurationFileLine("", verbose)
        ]

        configuration_file.migrate(RedemptionVersion("9.0.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_section_old_value_c: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("#old_value = 1234", verbose),
            ConfigurationFileLine("new_value = 1234", verbose),
            ConfigurationFileLine("", verbose),
            ConfigurationFileLine("", verbose)
        ]

        self.assertEqual(expected_content, configuration_file._content)
        self.assertEqual(len(expected_content),
            len(configuration_file._content))

    def test_migrate_to_9_1_0_section_old_value_d(self):
        verbose = False

        configuration_file = TestRedemptionConfigurationFile(None, verbose)

        configuration_file._content = [
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("", verbose),
            ConfigurationFileLine("old_value = 1234", verbose),
            ConfigurationFileLine("", verbose)
        ]

        configuration_file.migrate(RedemptionVersion("9.0.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_section_old_value_d: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("", verbose),
            ConfigurationFileLine("#old_value = 1234", verbose),
            ConfigurationFileLine("new_value = 1234", verbose),
            ConfigurationFileLine("", verbose)
        ]

        self.assertEqual(expected_content, configuration_file._content)
        self.assertEqual(len(expected_content),
            len(configuration_file._content))

    def test_migrate_to_9_1_0_section_old_value_2(self):
        verbose = False

        configuration_file = TestRedemptionConfigurationFile(None, verbose)

        configuration_file._content = [
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("old_value_2 = 1234", verbose)
        ]

        configuration_file.migrate(RedemptionVersion("9.0.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_section_old_value_2: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("#old_value_2 = 1234", verbose),
            ConfigurationFileLine("", verbose),
            ConfigurationFileLine("[new_section]", verbose),
            ConfigurationFileLine("new_value_2 = 1234", verbose)
        ]

        self.assertEqual(expected_content, configuration_file._content)
        self.assertEqual(len(expected_content),
            len(configuration_file._content))

    def test_migrate_to_9_1_0_section_old_value_2_b(self):
        verbose = False

        configuration_file = TestRedemptionConfigurationFile(None, verbose)

        configuration_file._content = [
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("old_value_2 = 1234", verbose),
            ConfigurationFileLine("[new_section]", verbose),
        ]

        configuration_file.migrate(RedemptionVersion("9.0.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_section_old_value_2_b: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("#old_value_2 = 1234", verbose),
            ConfigurationFileLine("[new_section]", verbose),
            ConfigurationFileLine("new_value_2 = 1234", verbose)
        ]

        self.assertEqual(expected_content, configuration_file._content)
        self.assertEqual(len(expected_content),
            len(configuration_file._content))

    def test_migrate_to_9_1_0_section_old_value_2_c(self):
        verbose = False

        configuration_file = TestRedemptionConfigurationFile(None, verbose)

        configuration_file._content = [
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("old_value_2 = 1234", verbose),
            ConfigurationFileLine("[new_section]", verbose),
            ConfigurationFileLine("", verbose)
        ]

        configuration_file.migrate(RedemptionVersion("9.0.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_section_old_value_2_c: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("#old_value_2 = 1234", verbose),
            ConfigurationFileLine("[new_section]", verbose),
            ConfigurationFileLine("new_value_2 = 1234", verbose),
            ConfigurationFileLine("", verbose)
        ]

        self.assertEqual(expected_content, configuration_file._content)
        self.assertEqual(len(expected_content),
            len(configuration_file._content))

    def test_migrate_to_9_1_0_section_old_value_2_d(self):
        verbose = False

        configuration_file = TestRedemptionConfigurationFile(None, verbose)

        configuration_file._content = [
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("old_value_2 = 1234", verbose),
            ConfigurationFileLine("[new_section]", verbose),
            ConfigurationFileLine("", verbose),
            ConfigurationFileLine("", verbose),
            ConfigurationFileLine("", verbose)
        ]

        configuration_file.migrate(RedemptionVersion("9.0.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_section_old_value_2_d: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("#old_value_2 = 1234", verbose),
            ConfigurationFileLine("[new_section]", verbose),
            ConfigurationFileLine("new_value_2 = 1234", verbose),
            ConfigurationFileLine("", verbose),
            ConfigurationFileLine("", verbose),
            ConfigurationFileLine("", verbose)
        ]

        self.assertEqual(expected_content, configuration_file._content)
        self.assertEqual(len(expected_content),
            len(configuration_file._content))

    def test_migrate_to_9_1_0_section_old_value_2_e(self):
        verbose = False

        configuration_file = TestRedemptionConfigurationFile(None, verbose)

        configuration_file._content = [
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("old_value_2 = 1234", verbose),
            ConfigurationFileLine("", verbose)
        ]

        configuration_file.migrate(RedemptionVersion("9.0.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_section_old_value_2_e: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("#old_value_2 = 1234", verbose),
            ConfigurationFileLine("", verbose),
            ConfigurationFileLine("[new_section]", verbose),
            ConfigurationFileLine("new_value_2 = 1234", verbose)
        ]

        self.assertEqual(expected_content, configuration_file._content)
        self.assertEqual(len(expected_content),
            len(configuration_file._content))

    def test_migrate_to_9_1_0_section_old_value_2_f(self):
        verbose = False

        configuration_file = TestRedemptionConfigurationFile(None, verbose)

        configuration_file._content = [
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("old_value_2 = 1234", verbose),
            ConfigurationFileLine("", verbose),
            ConfigurationFileLine("", verbose)
        ]

        configuration_file.migrate(RedemptionVersion("9.0.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_section_old_value_2_f: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("#old_value_2 = 1234", verbose),
            ConfigurationFileLine("", verbose),
            ConfigurationFileLine("[new_section]", verbose),
            ConfigurationFileLine("new_value_2 = 1234", verbose),
            ConfigurationFileLine("", verbose)
        ]

        self.assertEqual(expected_content, configuration_file._content)
        self.assertEqual(len(expected_content),
            len(configuration_file._content))

    def test_migrate_to_9_1_0_section_old_value_2_g(self):
        verbose = False

        configuration_file = TestRedemptionConfigurationFile(None, verbose)

        configuration_file._content = [
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("old_value_2 = 1234", verbose),
            ConfigurationFileLine("", verbose),
            ConfigurationFileLine("", verbose),
            ConfigurationFileLine("[new_section]", verbose),
            ConfigurationFileLine("", verbose)
        ]

        configuration_file.migrate(RedemptionVersion("9.0.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_section_old_value_2_g: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("#old_value_2 = 1234", verbose),
            ConfigurationFileLine("", verbose),
            ConfigurationFileLine("", verbose),
            ConfigurationFileLine("[new_section]", verbose),
            ConfigurationFileLine("new_value_2 = 1234", verbose),
            ConfigurationFileLine("", verbose)
        ]

        self.assertEqual(expected_content, configuration_file._content)
        self.assertEqual(len(expected_content),
            len(configuration_file._content))

    def test_migrate_to_9_1_0_from_8_2_0(self):
        verbose = False

        configuration_file = TestRedemptionConfigurationFile(None, verbose)

        configuration_file._content = [
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("value_8_2_0 = 1234", verbose),
            ConfigurationFileLine("", verbose)
        ]

        configuration_file.migrate(RedemptionVersion("8.2.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_from_8_2_0: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("#value_8_2_0 = 1234", verbose),
            ConfigurationFileLine("#old_value = 1234", verbose),
            ConfigurationFileLine("new_value = 1234", verbose),
            ConfigurationFileLine("", verbose)
        ]

        self.assertEqual(expected_content, configuration_file._content)
        self.assertEqual(len(expected_content),
            len(configuration_file._content))

    def test_migrate_to_9_1_0_from_8_2_0_b(self):
        verbose = False

        configuration_file = TestRedemptionConfigurationFile(None, verbose)

        configuration_file._content = [
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("value_2_8_2_0 = 1234", verbose),
            ConfigurationFileLine("", verbose)
        ]

        configuration_file.migrate(RedemptionVersion("8.2.0"))

        if verbose:
            print(
                 "test_migrate_to_9_1_0_from_8_2_0_b: "
                f"len(content)={len(configuration_file._content)}")

        expected_content = [
            ConfigurationFileLine("[section]", verbose),
            ConfigurationFileLine("#value_2_8_2_0 = 1234", verbose),
            ConfigurationFileLine("#old_value_2 = 1234", verbose),
            ConfigurationFileLine("", verbose),
            ConfigurationFileLine("[new_section]", verbose),
            ConfigurationFileLine("new_value_2 = 1234", verbose)
        ]

        self.assertEqual(expected_content, configuration_file._content)
        self.assertEqual(len(expected_content),
            len(configuration_file._content))
