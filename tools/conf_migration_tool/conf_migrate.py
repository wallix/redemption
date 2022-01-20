#!/usr/bin/python3 -O
# -*- coding: utf-8 -*-
##
# Copyright (c) 2021 WALLIX. All rights reserved.
# Licensed computer software. Property of WALLIX.
# Product Name: WALLIX Bastion v9.0
# Author(s): Raphael Zhou
# Id: $Id$
# URL: $URL$
# Module description:  Sesman Worker
##

from collections import OrderedDict
from shutil import copyfile

import os
import sys
import traceback

class RedemptionVersion:
    def __init__(self, version_string=None):
        self.__major = -1
        self.__minor = 0
        self.__build = 0
        self.__revision = ""

        if version_string:
            try:
                parts = version_string.split('.')

                if len(parts) != 3:
                    print(
                         "RedemptionVersion.__init__: "
                         "Invalid version string format: "
                        f"\"{version_string}\"")

                    return

                self.__major = int(parts[0])
                self.__minor = int(parts[1])

                revision_position =                                         \
                    self.__class__.__get_first_revision_letter_pos(parts[2])
                if -1 == revision_position:
                    self.__build = int(parts[2])
                else:
                    self.__build = int(parts[2][:revision_position])
                    self.__revision = parts[2][revision_position:]
            except Exception as e:
                _, exc_value, exc_traceback = sys.exc_info();
                print(
                     "RedemptionVersion.__init__: "
                    f"{type(exc_value).__name__}")
                traceback.print_tb(exc_traceback,
                    file=sys.stdout)

    def __str__(self):
        if -1 == self.__major:
            return "(invalid)"

        return                                                              \
            f"{self.__major}.{self.__minor}.{self.__build}{self.__revision}"

    def __gt__(self, other):
        if -1 == self.__major:
            if -1 == other.__major:
                return False

            return True
        elif -1 == other.__major:
            return False
        elif self.__major > other.__major:
            return True
        elif self.__major < other.__major:
            return False
        else:
            if self.__minor > other.__minor:
                return True
            elif self.__minor < other.__minor:
                return False
            else:
                if self.__build > other.__build:
                    return True
                elif self.__build < other.__build:
                    return False
                else:
                    if self.__revision > other.__revision:
                        return True
                    else:
                        return False

    def __lt__(self, other):
        if -1 == self.__major:
            return False
        if -1 == other.__major:
            return True
        elif self.__major < other.__major:
            return True
        elif self.__major > other.__major:
            return False
        else:
            if self.__minor < other.__minor:
                return True
            elif self.__minor > other.__minor:
                return False
            else:
                if self.__build < other.__build:
                    return True
                elif self.__build > other.__build:
                    return False
                else:
                    if self.__revision < other.__revision:
                        return True
                    else:
                        return False

    @classmethod
    def fromfile(cls, filename):
        version_string = ""

        try:
            with open(filename) as f:
                line = f.readline() # read first line

                items = line.split()

                version_string = items[1]

        except Exception as e:
            _, exc_value, exc_traceback = sys.exc_info();
            print(
                 "RedemptionVersion.fromfile: "
                f"{type(exc_value).__name__}")
            traceback.print_tb(exc_traceback,
                file=sys.stdout)

        return cls(version_string)

    @classmethod
    def __get_first_revision_letter_pos(cls, build_string):
        for i in range(0, len(build_string)):
            if build_string[i].isalpha():
                return i

        return -1

class ConfigurationFileLine:
    def __init__(self, raw_data, verbose = False):
        self.__raw_data = raw_data

        self.__verbose = verbose

        self.__is_comment = False
        self.__is_empty = False

        self.__is_section_declaration = False
        self.__name = None

        self.__is_variable_declaration = False
        self.__value = None

        self.__must_be_deleted = False

        striped_data = raw_data.strip()
        striped_data_length = len(striped_data)
        if 0 == striped_data_length:
           self.__is_empty = True
        elif '#' == striped_data[0]:
           self.__is_comment = True
        elif '[' == striped_data[0] and                                     \
             ']' == striped_data[striped_data_length - 1]:
           self.__is_section_declaration = True
           self.__name = striped_data[1:striped_data_length - 1].strip()
        else:
            equal_position = striped_data.find('=')
            if equal_position > -1:
                self.__is_variable_declaration = True
                self.__name = striped_data[:equal_position].strip()
                self.__value = striped_data[equal_position + 1:].strip()
            else:
                raise AssertionError(
                     "ConfigurationFile.__init__: Invalid format: "
                    f"\"{self.raw_data}\"")

    def __eq__(self, other):
        if self.__verbose:
            print(
                 "ConfigurationFileLine::__eq__: "
                f"self=\"{self.__raw_data}\" other=\"{other.__raw_data}\"")

        return self.__raw_data == other.__raw_data

    def __str__(self):
        return self.__raw_data

    def disable(self):
        if self.__is_variable_declaration:
            self.__raw_data = "#" + self.__raw_data

            self.__is_comment = True

            self.__is_section_declaration = False
            self.__name = None

            self.__is_variable_declaration = False
            self.__value = None

    def get_name(self):
        if not self.__is_section_declaration and                            \
           not self.__is_variable_declaration:
            raise AssertionError(
                 "ConfigurationFile.get_name: "
                 "Not a section declaration or a variable declaration: "
                f"\"{self.__raw_data}\"")

        return self.__name

    def get_value(self):
        if not self.__is_variable_declaration:
            raise AssertionError(
                 "ConfigurationFile.get_value: "
                f"Not a variable declaration: \"{self.__raw_data}\"")

        return self.__value

    def is_comment(self):
        return self.__is_comment

    def is_empty(self):
        return self.__is_empty

    def is_marked_to_be_deleted(self):
        if self.__verbose:
            print(
                 "ConfigurationFileLine::is_marked_to_be_deleted: "
                f"\"{self.__raw_data}\" MustBeDeleted={'Yes' if self.__must_be_deleted else 'No'}")

        return self.__must_be_deleted

    def is_section_declaration(self):
        return self.__is_section_declaration

    def is_variable_declaration(self):
        return self.__is_variable_declaration

    def mark_to_be_deleted(self):
        if self.__verbose:
            print(
                 "ConfigurationFileLine::mark_to_be_deleted: "
                f"\"{self.__raw_data}\"")

        if self.__is_comment:
            self.__must_be_deleted = True

class ConfigurationFile:
    def __init__(self, filename = None, verbose = False):
        self._content = []
        self.__filename = None

        self.__verbose = verbose

        if filename:
            try:
                f = open(filename, 'r', encoding='utf-8')

                self.__filename = filename

                while True:
                    line_raw_data = f.readline()
                    if not line_raw_data:
                        break

                    self._content.append(
                        ConfigurationFileLine(line_raw_data, self.__verbose))

                f.close()
            except Exception as e:
                _, exc_value, exc_traceback = sys.exc_info();
                print(
                     "ConfigurationFile.__init__: "
                    f"{type(exc_value).__name__}")
                traceback.print_tb(exc_traceback,
                    file=sys.stdout)

    def __add_variable(self, section_name, line_raw_data):
        """
        Retourne -1 et 0 si une variable qui porte le même nom existe déjà
        dans la section. Déclenche une assertion si la line_raw_data
        represente une nouvelle section. Sinon, ajouter une nouvelle ligne
        vide a la fin de la section concernée, puis ajouter la ligne concernée
        en-dessous, ensuite retourner la position de la premier ligne ajoutée
        (ligne vide) ainsi que le nombre de lignes ajoutées.
        """

        inserted_line_count = 0

        configuration_file_line = ConfigurationFileLine(
            line_raw_data, self.__verbose)

        if configuration_file_line.is_section_declaration():
            raise AssertionError(
                 "ConfigurationFile.__add_variable: "
                f"Cannot add a new section: \"{line_raw_data}\"")

        if not section_name:
            raise AssertionError(
                 "ConfigurationFile.__add_variable: "
                f"Section name is invalid or missing: \"{line_raw_data}\"")

        if configuration_file_line.is_variable_declaration():
            if self.__is_variable_exist(section_name,
                                        configuration_file_line.get_name()):
                print("ConfigurationFile.__add_variable: "
                     "A variable of the same name still exists in the "
                         "section: "
                    f"\"{str(configuration_file_line)}\"")

                return -1, 0

        insert_position, ins_pos = self.__find_section_append_pos(section_name)
        inserted_line_count += ins_pos

        self._content.insert(insert_position,
            ConfigurationFileLine(line_raw_data, self.__verbose))
        inserted_line_count += 1

        if len(self._content) > insert_position + 1 and                     \
           not self._content[insert_position + 1].is_empty():
            if self.__verbose:
                print("ConfigurationFile.__add_variable: Insert blank line")

            self._content[insert_position + 1].insert(insert_position,
                ConfigurationFileLine("", self.__verbose))
            inserted_line_count += 1

        return insert_position, inserted_line_count

    def __find_section_append_pos(self, section_name):
        """
        Retourne la position d'ajout de nouvelle élément dans une section.
        La section sera ajoutée à la fin du fichier si elle n'existe pas.
        La méthode assure qu'il y a une ligne vide devant la position
        d'insertion si la section contient d'autres valeurs.
        """

        append_position = -1
        appended_line_count = 0

        line_count = len(self._content)
        line_index = 0
        section_found = False
        in_section = False
        while line_index < line_count:
            if self._content[line_index].is_section_declaration():
                in_section =                                                \
                    (self._content[line_index].get_name() == section_name)
                if in_section:
                    section_found = True
                    append_position = line_index + 1
            elif self._content[line_index].is_variable_declaration() or     \
                 self._content[line_index].is_empty() or                    \
                 self._content[line_index].is_comment():
                if in_section:
                    append_position = line_index + 1

            line_index += 1

        if -1 == append_position:
            append_position = line_count

        while append_position > 1 and                                       \
              self._content[append_position - 1].is_empty() and             \
              (self._content[append_position - 2].is_empty() or             \
               self._content[append_position - 2].is_section_declaration()):
            append_position -= 1

        if not self._content[append_position - 1].is_empty() and            \
           not self._content[append_position - 1].is_section_declaration():
            if self.__verbose:
                print("ConfigurationFile.__find_section_append_pos: "
                    "Insert blank line")

            self._content.insert(append_position,
                ConfigurationFileLine("", self.__verbose))

            append_position += 1
            appended_line_count += 1

        if not section_found:
            self._content.insert(append_position,
                ConfigurationFileLine(f"[{section_name}]", self.__verbose))
            append_position += 1
            appended_line_count += 1

        return append_position, appended_line_count

    def save(self):
        save_to(self.__filename)

    def save_to(self, filename):
        if filename:
            try:
                f = open(filename, 'w', encoding='utf-8')

                for line in self._content:
                    if line.is_marked_to_be_deleted():
                        continue

                    line_raw_data = str(line).rstrip("\r\n") + "\n"
                    f.write(f'{line_raw_data}')

                f.close()
            except Exception as e:
                _, exc_value, exc_traceback = sys.exc_info();
                print(
                     "ConfigurationFile.save_to: "
                    f"{type(exc_value).__name__}")
                traceback.print_tb(exc_traceback,
                    file=sys.stdout)

    def migrate(self, previous_version):
        first_migration = True

        while True:
            line_migration_func, result_version =                           \
                self._get_line_migration_func(previous_version)
            if line_migration_func:
                line_count = len(self._content)
                line_index = 0
                section_name = None
                while line_index < line_count:
                    if self._content[line_index].is_section_declaration():
                        section_name = self._content[line_index].get_name()
                    elif self._content[line_index].is_variable_declaration():
                        if not section_name:
                            raise AssertionError(
                                 "ConfigurationFile.migrate: "
                                 "Not in a section: "
                                f"\"{str(configuration_file_line)}\"")

                        keep_unchanged, dest_section_name, line_raw_data =  \
                            line_migration_func(section_name,
                                self._content[line_index])
                        if not keep_unchanged:
                            self._content[line_index].disable()

                            if not first_migration:
                                self._content[line_index].mark_to_be_deleted()

                            if dest_section_name:
                                insert_position, insert_count =             \
                                    self.__add_variable(
                                        dest_section_name,
                                        line_raw_data)

                                if -1 < insert_position:
                                    if 1 > insert_count:
                                        raise AssertionError(
                                             "ConfigurationFile.migrate: "
                                             "Invalid return values: "
                                             "(insert_position="
                                            f"{insert_position}, "
                                            f"insert_count={insert_count})")

                                    if insert_position == line_index:
                                        raise AssertionError(
                                             "ConfigurationFile.migrate: "
                                             "Invalid insert position: "
                                             "(insert_position="
                                                f"{insert_position}, "
                                                f"insert_count="
                                                 "{insert_count})")

                                    if insert_position < line_index:
                                        line_index += insert_count

                                    line_count += insert_count
                            elif line_raw_data:
                                configuration_file_line =                   \
                                    ConfigurationFileLine(line_raw_data,
                                        self.__verbose)

                                if configuration_file_line.is_section_declaration():
                                    raise AssertionError(
                                         "ConfigurationFile.migrate: "
                                         "Should not insert a new section "
                                             "with this method: "
                                        f"\"{str(configuration_file_line)}\"")

                                if configuration_file_line.is_variable_declaration() and \
                                   self.__is_variable_exist(
                                        dest_section_name,
                                        configuration_file_line.get_name()):
                                    print(
                                         "ConfigurationFile.migrate: "
                                         "A variable of the same name still "
                                             "exists in the section: "
                                        f"\"{str(configuration_file_line)}\"")
                                else:
                                    self._content.insert(line_index + 1,
                                        configuration_file_line)

                                    line_count += 1
                                    line_index += 1

                    line_index += 1

                previous_version = result_version
            else:
                break

            first_migration = False

        line_count = len(self._content)
        line_index = 0
        while line_index < line_count:
            if self._content[line_index].is_marked_to_be_deleted():
                self._content.pop(line_index)

                line_count -= 1

                continue

            line_index += 1

    def _get_line_migration_func(self, previous_version):
        raise NotImplementedError(
            "ConfigurationFile._get_line_migration_func: "
            "Must override _get_line_migration_func")

    def __is_variable_exist(self, section_name, variable_name):
        """
        Retourne False si la variable n'existe pas dans la section, ou si la
            section n'existe pas. Sinon retourne True.
        """

        line_count = len(self._content)
        line_index = 0
        current_section_name = None
        while line_index < line_count:
            if self._content[line_index].is_section_declaration():
                current_section_name = self._content[line_index].get_name()
            elif self._content[line_index].is_variable_declaration():
                if not current_section_name:
                    raise AssertionError(
                         "ConfigurationFile.__is_variable_exist: "
                         "Not in a section: "
                        f"\"{str(configuration_file_line)}\"")

                if current_section_name == section_name and                 \
                   self._content[line_index].get_name() == variable_name:
                    return True

            line_index += 1

        return False

class RedemptionConfigurationFile(ConfigurationFile):
    def _get_line_migration_func(self, previous_version):
        noneable_line_migration_func = None
        noneable_result_version = None

        v_9_1_39 = RedemptionVersion("9.1.39")
        if previous_version < v_9_1_39:
            noneable_line_migration_func = self.__migrate_line_to_9_1_39
            noneable_result_version = v_9_1_39

        return noneable_line_migration_func, noneable_result_version

    def __migrate_line_to_9_1_39(self, section_name, line):
        keep_unchanged = True
        noneable_dest_section_name = None
        noneable_line_raw_data = None

        if line.is_variable_declaration():
            if "globals" == section_name:
                if "session_timeout" == line.get_name():
                    keep_unchanged = False
                    noneable_dest_section_name = None
                    noneable_line_raw_data =                                \
                        f"base_inactivity_timeout = {line.get_value()}"

        return keep_unchanged, noneable_dest_section_name,                  \
            noneable_line_raw_data


if os.path.exists('/tmp/OLD_REDEMPTION_VERSION') and                        \
   os.path.exists('/var/wab/etc/rdp/rdpproxy.ini'):
    old_redemption_version =                                                \
        RedemptionVersion.fromfile('/tmp/OLD_REDEMPTION_VERSION')
    print(f"PreviousRedemptionVersion={old_redemption_version}")

    copyfile('/var/wab/etc/rdp/rdpproxy.ini',                               \
        '/var/wab/etc/rdp/rdpproxy.ini.work')

    new_configuration_file =                                                \
        RedemptionConfigurationFile('/var/wab/etc/rdp/rdpproxy.ini.work')
    new_configuration_file.migrate(old_redemption_version)
    new_configuration_file.save_to('/var/wab/etc/rdp/rdpproxy.ini.work')

    copyfile('/var/wab/etc/rdp/rdpproxy.ini',                               \
        '/var/wab/etc/rdp/rdpproxy.ini.bak')

    os.rename('/var/wab/etc/rdp/rdpproxy.ini.work',                         \
        '/var/wab/etc/rdp/rdpproxy.ini')
