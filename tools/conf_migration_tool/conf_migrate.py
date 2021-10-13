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

def get_first_revision_letter_pos(build_string):
    for i in range(0, len(build_string)):
        if build_string[i].isalpha():
            return i

    return -1

def get_redemption_version_from_string(version_string):
    version = {
        "major": -1,
        "minor": 0,
        "build": 0,
        "revision": ""
    }

    try:
        parts = version_string.split('.')

        if len(parts) != 3:
            raise Exception()

        version["major"] = int(parts[0])
        version["minor"] = int(parts[1])

        revision_position = get_first_revision_letter_pos(parts[2])
        if revision_position == -1:
            version["build"] = int(parts[2])
            version["revision"] = ""
        else:
            version["build"] = int(parts[2][:revision_position])
            version["revision"] = parts[2][revision_position:]
    except Exception:
        pass

    return version

def get_redemption_version_from_file(file_path):
    version_string = ""

    try:
        with open(file_path) as f:
            line = f.readline()   # read first line

            items = line.split()

            version_string = items[1]

    except Exception:
        pass

    return get_redemption_version_from_string(version_string)



# print(get_redemption_version_from_file("/tmp/REDEMPTION_VERSION"))

# print(get_redemption_version_from_string("3.5.9"))

