#!/usr/bin/python -O
# -*- coding: utf-8 -*-
#

"""
Disk module (to load and store data in the wab hard drive disk)
"""

import Image
import itertools as its
import struct
import base64
from os.path import exists

from redcrypto.error import CryptoError

class Disk:
    """ The disk class """

    def __init__(self, base_path):
        """ Constructor with base_path: the directory holding the data's medium (logo.png) """
        if not exists(base_path):
            raise CryptoError("%s: No such directory" % base_path)
        self.medium = "%s/%s.p" % (base_path, "logo")

    def initialize(self):
        """ Initialize the medium """
        if exists("%sng" % self.medium):
            raise CryptoError("Disk is already initialized!")
        open("%sng" % self.medium, "wb").write(base64.decodestring(MEDIUM))

    def load(self):
        """ Read data from the medium """
        if not exists("%sng" % self.medium):
            raise CryptoError("Disk isn't initialized!")
        bits = (cc & 1 for px in Image.open("%sng" % self.medium).getdata() for cc in px)
        acc = lambda n: reduce(lambda a, b: a << 1 | b, its.islice(bits, n), 0)
        return ''.join(its.imap(lambda x: x(), its.repeat(lambda: chr(acc(8)), acc(32))))

    def store(self, data):
        """ Store data to the medium """
        get_bits = lambda items, n, fillvalue: its.izip_longest(*[iter(items)] * n, fillvalue=fillvalue)
        biterator = lambda d: ((ord(ch) >> shift) & 1 for ch, shift in its.product(d, range(7, -1, -1)))
        magic = lambda pixel, bits: tuple(its.starmap(lambda cpt, bit: cpt & ~1 | bit, zip(pixel, bits)))
        img = Image.open("%sng" % self.medium)
        bits = get_bits(biterator('%s%s' % (struct.pack(">i", len(data)), data)), len(img.getbands()), 0)
        img.putdata(list(its.starmap(magic, its.izip(img.getdata(), bits))))
        img.save("%sng" % self.medium)

    def destroy(self):
        """ Remove persistant disk storage """
        import os
        if exists("%sng" % self.medium):
            os.unlink("%sng" % self.medium)

MEDIUM = (
"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"
)
