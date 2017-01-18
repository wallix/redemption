#!/usr/bin/python -O
# -*- coding: utf-8 -*-
#

import redcryptofile

print redcryptofile.Random().rand()

class Random2(redcryptofile.Random):
    def __init__(self, item):
        self.item = item

r = Random2(1)

print r.rand()

