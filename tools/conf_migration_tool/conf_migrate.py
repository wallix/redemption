#!/usr/bin/python3 -O
# -*- coding: utf-8 -*-
##
# Copyright (c) 2021 WALLIX. All rights reserved.
# Licensed computer software. Property of WALLIX.
# Product Name: WALLIX Bastion v9.0
# Author(s): Raphael Zhou
# Module description:  Sesman Worker
##

from collections import OrderedDict
from shutil import copyfile
from enum import Enum
from typing import List, Tuple, Optional, Union, Callable, Iterable

import os
import re
import sys
import traceback

rgx_version = re.compile('^(\d+)\.(\d+)\.(\d+)(.*)')

class RedemptionVersionError(Exception):
    pass

class RedemptionVersion:
    def __init__(self, version:str) -> None:
        m = rgx_version.match(version)
        if m is None:
            raise RedemptionVersionError("RedemptionVersion: "
                                         "Invalid version string format: "
                                         + version)

        self.__major = int(m.group(1))
        self.__minor = int(m.group(2))
        self.__build = int(m.group(3))
        self.__revision = m.group(4)

    def __str__(self) -> str:
        return f"{self.__major}.{self.__minor}.{self.__build}{self.__revision}"

    def __lt__(self, other:'RedemptionVersion') -> bool:
        return self.__part() < other.__part()

    def __le__(self, other:'RedemptionVersion') -> bool:
        return self.__part() <= other.__part()

    @staticmethod
    def from_file(filename:str) -> 'RedemptionVersion':
        with open(filename) as f:
            line = f.readline() # read first line
            items = line.split()
            version_string = items[1]
            return RedemptionVersion(version_string)

    def __part(self) -> Tuple[int,int,int,str]:
        return (self.__major, self.__minor, self.__build, self.__revision)

class ConfigLineKind(Enum):
    Empty = 1
    Comment = 2
    SectionDecl = 3
    VariableDecl = 4

class ConfigurationLine:
    def __init__(self, raw_data:str, verbose:bool=False) -> None:
        self.__raw_data = raw_data

        self.__verbose = verbose

        self.__name = None
        self.__value = None
        self.__must_be_deleted = False

        striped_data = raw_data.strip()

        if not striped_data:
            self.__kind = ConfigLineKind.Empty
        elif '#' == striped_data[0]:
            self.__kind = ConfigLineKind.Comment
        elif '[' == striped_data[0] and ']' == striped_data[-1]:
            self.__kind = ConfigLineKind.SectionDecl
            self.__name = striped_data[1:-1].strip()
        else:
            name, sep, value = striped_data.partition('=')
            assert sep, f'Invalid format: \"{self.raw_data}\"'
            self.__name = name.strip()
            self.__value = value.strip()
            self.__kind = ConfigLineKind.VariableDecl

    def __eq__(self, other:'ConfigurationLine') -> bool:
        if self.__verbose:
            print("ConfigurationLine::__eq__: "
                  f"self=\"{self.__raw_data}\" other=\"{other.__raw_data}\"")

        return self.__raw_data == other.__raw_data

    def __str__(self) -> str:
        return self.__raw_data

    def disable(self) -> None:
        if self.__kind == ConfigLineKind.VariableDecl:
            self.__raw_data = "#" + self.__raw_data
            self.__kind = ConfigLineKind.Comment

            self.__name = None
            self.__value = None

    def get_name(self) -> str:
        assert self.__kind == ConfigLineKind.SectionDecl \
            or self.__kind == ConfigLineKind.VariableDecl

        return self.__name

    def get_value(self) -> str:
        assert self.__kind == ConfigLineKind.VariableDecl

        return self.__value

    def is_comment(self) -> bool:
        return self.__kind == ConfigLineKind.Comment

    def is_empty(self) -> bool:
        return self.__kind == ConfigLineKind.Empty

    def is_marked_to_be_deleted(self) -> bool:
        if self.__verbose:
            print("ConfigurationLine::is_marked_to_be_deleted: "
                 f"\"{self.__raw_data}\" MustBeDeleted={'Yes' if self.__must_be_deleted else 'No'}")

        return self.__must_be_deleted

    def is_section_declaration(self) -> bool:
        return self.__kind == ConfigLineKind.SectionDecl

    def is_variable_declaration(self) -> bool:
        return self.__kind == ConfigLineKind.VariableDecl

    def mark_to_be_deleted(self) -> None:
        if self.__verbose:
            print("ConfigurationLine::mark_to_be_deleted: "
                 f"\"{self.__raw_data}\"")

        if self.__kind == ConfigLineKind.Comment:
            self.__must_be_deleted = True

def read_configuration_lines(filename:str=None, verbose:bool=False) -> List[ConfigurationLine]:
    with open(filename, encoding='utf-8') as f:
        return [ConfigurationLine(line, verbose) for line in f]

class ConfigurationFile:
    def __init__(self, content:List[ConfigurationLine], verbose:bool=False) -> None:
        self._content = content
        self.__verbose = verbose

    def __add_variable(self, section_name:str, line_raw_data:str) -> Tuple[int,int]:
        """
        Retourne -1 et 0 si une variable qui porte le même nom existe déjà
        dans la section. Déclenche une assertion si la line_raw_data
        represente une nouvelle section. Sinon, ajouter une nouvelle ligne
        vide a la fin de la section concernée, puis ajouter la ligne concernée
        en-dessous, ensuite retourner la position de la premier ligne ajoutée
        (ligne vide) ainsi que le nombre de lignes ajoutées.
        """

        configuration_file_line = ConfigurationLine(line_raw_data, self.__verbose)

        assert not configuration_file_line.is_section_declaration(), \
            f'Cannot add a new section: "{line_raw_data}"'
        assert section_name, f'Section name is invalid or missing: "{line_raw_data}"'

        if configuration_file_line.is_variable_declaration():
            if self.__is_variable_exist(section_name,
                                        configuration_file_line.get_name()):
                print("ConfigurationFile.__add_variable: "
                      "A variable of the same name still exists in the "
                     f"section: \"{configuration_file_line}\"")

                return -1, 0

        insert_position, inserted_line_count = self.__find_section_append_pos(section_name)

        self._content.insert(insert_position,
            ConfigurationLine(line_raw_data, self.__verbose))
        inserted_line_count += 1

        return insert_position, inserted_line_count

    def __find_section_append_pos(self, section_name:str) -> Tuple[str,str]:
        """
        Retourne la position d'ajout de nouvelle élément dans une section.
        La section sera ajoutée à la fin du fichier si elle n'existe pas.
        La méthode assure qu'il y a une ligne vide devant la position
        d'insertion si la section contient d'autres valeurs.
        """

        append_position = -1
        appended_line_count = 0

        line_index = 0
        section_found = False
        in_section = False
        for line_index, line in enumerate(self._content):
            if line.is_section_declaration():
                in_section = (line.get_name() == section_name)
                if in_section:
                    section_found = True
                    append_position = line_index + 1
            elif line.is_variable_declaration() or     \
                 line.is_empty() or                    \
                 line.is_comment():
                if in_section:
                    append_position = line_index + 1

        if -1 == append_position:
            append_position = len(self._content)

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
                ConfigurationLine("", self.__verbose))

            append_position += 1
            appended_line_count += 1

        if not section_found:
            self._content.insert(append_position,
                ConfigurationLine(f"[{section_name}]", self.__verbose))
            append_position += 1
            appended_line_count += 1

        return append_position, appended_line_count

    def save_to(self, filename:str) -> None:
        with open(filename, 'w', encoding='utf-8') as f:
            f.write('\n'.join(
                str(line).rstrip("\r\n")
                for line in self._content
                if not line.is_marked_to_be_deleted()
            ))
            f.write('\n')

    def migrate(self, migration_defs:Iterable[Tuple[RedemptionVersion,
                                                    Callable[[str, str],
                                                             Optional[Tuple[Optional[str],
                                                                            str]]]]],
                previous_version:RedemptionVersion) -> None:
        content_is_changed = False

        first_round_of_migration = True

        migration_defs = sorted(migration_defs, key=lambda t: t[0])

        for result_version, line_migration_func in migration_defs:
            if previous_version >= result_version:
                continue

            line_count = len(self._content)
            line_index = 0
            section_name = None
            while line_index < line_count:
                if self._content[line_index].is_section_declaration():
                    section_name = self._content[line_index].get_name()
                elif self._content[line_index].is_variable_declaration():
                    assert section_name, f'Not in a section: "{configuration_file_line}"'

                    migrate_result = line_migration_func(
                        section_name, self._content[line_index])

                    if migrate_result:
                        dest_section_name, line_raw_data = migrate_result

                        content_is_changed = True

                        self._content[line_index].disable()

                        if not first_round_of_migration:
                            self._content[line_index].mark_to_be_deleted()

                        if dest_section_name:
                            insert_position, insert_count =             \
                                self.__add_variable(
                                    dest_section_name,
                                    line_raw_data)

                            if -1 < insert_position:
                                assert 1 <= insert_count, \
                                    "Invalid return values: " \
                                    "(insert_position=" \
                                    f"{insert_position}, " \
                                    f"insert_count={insert_count})"

                                assert insert_position != line_index, \
                                    "Invalid insert position: " \
                                    "(insert_position=" \
                                    f"{insert_position}, " \
                                    f"insert_count={insert_count})"

                                if insert_position < line_index:
                                    line_index += insert_count

                                line_count += insert_count
                        else:
                            configuration_file_line =                   \
                                ConfigurationLine(line_raw_data,
                                    self.__verbose)

                            assert not configuration_file_line.is_section_declaration(), \
                                "Should not insert a new section with this method: " \
                                f"\"{configuration_file_line}\""

                            if configuration_file_line.is_variable_declaration() and \
                                self.__is_variable_exist(
                                    dest_section_name,
                                    configuration_file_line.get_name()):
                                print("ConfigurationFile.migrate: "
                                      "A variable of the same name still "
                                      "exists in the section: "
                                     f"\"{configuration_file_line}\"")
                            else:
                                self._content.insert(line_index + 1,
                                    configuration_file_line)

                                line_count += 1
                                line_index += 1

                line_index += 1

            previous_version = result_version

            first_round_of_migration = False

        self._content = [line for line in self._content
                         if not line.is_marked_to_be_deleted()]

        return content_is_changed

    def __is_variable_exist(self, section_name:Optional[str], variable_name:str) -> bool:
        """
        Retourne False si la variable n'existe pas dans la section, ou si la
            section n'existe pas. Sinon retourne True.
        """

        current_section_name = None
        for line in self._content:
            if line.is_section_declaration():
                current_section_name = line.get_name()
            elif line.is_variable_declaration():
                assert current_section_name, \
                    f'Not in a section: "{configuration_file_line}"'

                if current_section_name == section_name and                 \
                   line.get_name() == variable_name:
                    return True

        return False


def migrate_line_to_9_1_39(self, section_name:str, line:ConfigurationLine) -> Optional[Tuple[Optional[str], str]]:
    if line.is_variable_declaration():
        if "globals" == section_name:
            if "session_timeout" == line.get_name():
                return None, f"base_inactivity_timeout = {line.get_value()}"

def migrate_line_to_9_1_71(self, section_name:str, line:ConfigurationLine) -> Optional[Tuple[Optional[str], str]]:
    if line.is_variable_declaration():
        if 'rdp' == section_name:
            varname = line.get_name()
            if varname in (
                'session_probe_exe_or_file',
                'session_probe_arguments',
                'session_probe_customize_executable_name',
                'session_probe_allow_multiple_handshake',
                'session_probe_at_end_of_session_freeze_connection_and_wait',
                'session_probe_enable_cleaner',
                'session_probe_clipboard_based_launcher_reset_keyboard_status',
            ):
                return 'session_probe', f'{varname[14:]} = {line.get_value()}'

            if varname == 'session_probe_bestsafe_integration':
                return 'session_probe', 'enable_bestsafe_interaction'

        elif 'video' == section_name:
            if 'replay_path' == line.get_name():
                return 'mod_replay', f'replay_path = {line.get_value()}'

def migrate_line_to_9_1_76(self, section_name:str, line:ConfigurationLine) -> Optional[Tuple[Optional[str], str]]:
    if line.is_variable_declaration():
        if "all_target_mod" == section_name:
            if "connection_retry_count" == line.get_name():
                return None, None

migration_defs = (
    (RedemptionVersion("9.1.39"), migrate_line_to_9_1_39),
    (RedemptionVersion("9.1.71"), migrate_line_to_9_1_71),
    (RedemptionVersion("9.1.76"), migrate_line_to_9_1_76),
)


if __name__ == '__main__':
    if os.path.exists('/tmp/OLD_REDEMPTION_VERSION') and                    \
       os.path.exists('/var/wab/etc/rdp/rdpproxy.ini'):
        try:
            old_redemption_version = RedemptionVersion.from_file(
                '/tmp/OLD_REDEMPTION_VERSION')

            print(f"PreviousRedemptionVersion={old_redemption_version}")

            lines = read_configuration_lines('/var/wab/etc/rdp/rdpproxy.ini')

            new_configuration_file = ConfigurationFile(lines)

            if new_configuration_file.migrate(migration_defs, old_redemption_version):
                new_configuration_file.save_to('/var/wab/etc/rdp/rdpproxy.ini.work')

                copyfile('/var/wab/etc/rdp/rdpproxy.ini',
                        f'/var/wab/etc/rdp/rdpproxy.ini.{old_redemption_version}')

                os.rename('/var/wab/etc/rdp/rdpproxy.ini.work',
                          '/var/wab/etc/rdp/rdpproxy.ini')

                print("Configuration file updated")
        except Exception as e:
            _, exc_value, exc_traceback = sys.exc_info()
            print(type(exc_value).__name__)
            traceback.print_tb(exc_traceback, file=sys.stdout)
