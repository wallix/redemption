#! /usr/bin/env python
# -*- coding: utf-8 -*-

import re
import platform

def get_distro():
    f = open("/etc/issue")
    issue = f.read(50)
    f.close()
    res = re.match(r"([A-Za-z]+)[^0-9]+([0-9]+([.][0-9]+)?)", issue)
    dist, version, name, arch = "Unknown", "0", "unknown", "comodore"
    if res:
        dist, version = res.group(1), res.group(2)
        if dist == 'Debian':
            if version in ['5', '5.0']:
                version = '5'
                name = 'lenny'
            elif version in ['6', '6.0']:
                version = '6'
                name = 'squeeze'
            elif version in ['7', '7.0']:
                version = '7'
                name = 'wheezy'
        elif dist == 'Ubuntu':
            codenames = {
                '4.10': ('Warty', 'Warthog'),
                '5.04': ('Hoary', 'Hedgehog'),
                '5.10': ('Breezy', 'Badger'),
                '6.06': ('Dapper', 'Drake'),
                '6.10': ('Edgy', 'Eft'),
                '7.04': ('Feisty', 'Fawn'),
                '7.10': ('Gutsy', 'Gibbon'),
                '8.04': ('Hardy', 'Heron'),
                '8.10': ('Intrepid', 'Ibex'),
                '9.04': ('Jaunty', 'Jackalope'),
                '9.10': ('Karmic', 'Koala'),
                '10.04': ('Lucid', 'Lynx'),
                '10.10': ('Maverick', 'Meerkat'),
                '11.04': ('Natty', 'Narwhal'),
                '11.10': ('Oneiric', 'Ocelot'),
                '12.04': ('Precise', 'Pangolin'),
                '12.10': ('Quantal', 'Quetzal'),
                '13.04': ('Raring', 'Ringtail'),
                '13.10': ('Saucy', 'Salamander'),
                '14.04': ('Trusty', 'Tahr'),
                '14.10': ('Utopic', 'Unicorn'),
                '15.04': ('Vivid', 'Vervet'),
                '15.10': ('Wily', 'Werewolf'),
                '16.04': ('Xenial', 'Xerus'),
                '16.10': ('Yakketi', 'Yak'),
                '17.04': ('Zesty', 'Zapus'),
                '17.10': ('Artful', 'Aardvark'),
                '18.04': ('B', 'B'),
                '18.10': ('C', 'C')
            }
            name = codenames[version][0].lower()
    else:
        words = re.split("\W", issue)
        if "lenny" in words:
            dist, version, name = "debian", "5", "lenny"
        elif "squeeze" in words:
            dist, version, name = "debian", "6", "squeeze"
        elif "wheezy" in words:
            dist, version, name = "debian", "7", "wheezy"

    return dist.lower(), version, name


def get_arch():
    if (platform.uname()[4] == 'x86_64'):
        arch = "amd64"
    else:
        arch = "i386"
    return arch

