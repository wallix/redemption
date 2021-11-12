#!/usr/bin/python -O
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

import sys
import traceback

class ExecutableVersion:
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
                         "ExecutableVersion.__init__: "
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
                     "ExecutableVersion.__init__: "
                    f"{type(exc_value).__name__}")
                traceback.print_tb(exc_traceback,
                    file=sys.stdout)

    def __str__(self):
        if -1 == self.__major:
            return "(invalid)"

        return f"{self.__major}.{self.__minor}.{self.__build}{self.__revision}"

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
                 "ExecutableVersion.fromfile: "
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
    def __init__(self, row_data):
        self.__raw_data = row_data

    def __str__(self):
        return self.__raw_data

def load_configuration_file(filename):
    file_content = []
#    print("load_configuration_file")

    f = open(filename, 'r', encoding='utf-8')
    while True:
        line_raw_data = f.readline()
        if not line_raw_data:
            break

#        print(line_raw_data)

        file_content.append(ConfigurationFileLine(line_raw_data))

#    print(file_content)
    f.close()

    return file_content

def save_configuration_file(filename, file_content):
    f = open(filename, 'w', encoding='utf-8')

    for line in file_content:
        f.write(f'{str(line)}')

    f.close()
