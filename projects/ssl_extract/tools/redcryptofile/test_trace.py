#!/usr/bin/python -O
# -*- coding: utf-8 -*-
#

import os
PATH = os.path.dirname(os.path.realpath(__file__))

from redcryptotrace import CryptoTrace, Trace

import hashlib


def hashfile(fname):
    blocksize = 65536
    hasher = hashlib.md5()
    with open(fname) as f:
        buf = f.read(blocksize)
        while len(buf) > 0:
            hasher.update(buf)
            buf = f.read(blocksize)
    return ''.join([hex(256+ord(x))[-2:].upper() for x in hasher.digest()])


import unittest
import os

class TestCryptoTrace(unittest.TestCase):

    def setUp(self):
        for f in ["text.txt.clear.trc", "text.txt.clear.roundtrip",
                  "long.bin.clear.trc", "long.bin.clear.roundtrip",
                  "text.txt.crypted.trc", "text.txt.roundtrip",
                  "long.bin.crypted.trc", "long.bin.roundtrip"]:
            try:
                os.unlink(os.path.join(PATH,f))
            except:
                pass

    def tearDown(self):
        """cleanup test files"""
        for f in ["text.txt.clear.trc", "text.txt.clear.roundtrip",
                  "long.bin.clear.trc", "long.bin.clear.roundtrip",
                  "text.txt.crypted.trc", "text.txt.roundtrip",
                  "long.bin.crypted.trc", "long.bin.roundtrip"]:
            try:
                os.unlink(os.path.join(PATH,f))
            except:
                pass

    def test_decrypttextfile(self):
        t2 = CryptoTrace(os.path.join(PATH,"fixtures/text.txt.trc"), "r")
        with open(os.path.join(PATH,"text.txt.roundtrip"), 'w') as f:
            while True:
                data = t2.read(1024)
                if len(data) == 0:
                    break
                print data
                f.write(data)
        t2.close()

        # check decrypted file on disk same as original
        self.assertEquals('1F439C7547B1EF1C1D3085483A87AD2E', 
            hashfile(os.path.join(PATH,"text.txt.roundtrip")))


    def test_decryptlongfile(self):
        t2 = CryptoTrace(os.path.join(PATH,"fixtures/long.bin.trc"), "r")
        with open(os.path.join(PATH,"long.bin.roundtrip"), 'w') as f:
            while True:
                data = t2.read(1024)
                if len(data) == 0:
                    break
                f.write(data)
            t2.close()

        # check decrypted file on disk same as original
        self.assertEquals('36209641806DB628D04F239192CD8E8C', 
            hashfile(os.path.join(PATH,"long.bin.roundtrip")))


    def test_crypttextfile(self):
        t = CryptoTrace(os.path.join(PATH,"text.txt.crypted.trc"), "w")
        with open(os.path.join(PATH,"fixtures/text.txt"), 'r') as f:
            for l in f:
                t.write(l)
        t.flush()
        t.close()
        
        # check encrypted file on disk did not change
        self.assertEquals('995EB32D1067C5681B386E58305D14B5', 
            hashfile(os.path.join(PATH,"text.txt.crypted.trc")))

        # check round trip
        t2 = CryptoTrace(os.path.join(PATH,"text.txt.crypted.trc"), "r")
        with open(os.path.join(PATH,"text.txt.roundtrip"), 'w') as f:
            while True:
                data = t2.read(1024)
                if len(data) == 0:
                    break
                f.write(data)
        t2.close()

        # check decrypted file on disk same as original
        self.assertEquals('1F439C7547B1EF1C1D3085483A87AD2E',
            hashfile(os.path.join(PATH,"text.txt.roundtrip")))

    def test_cryptlongfile(self):
        t = CryptoTrace(os.path.join(PATH,"long.bin.crypted.trc"), "w")
        with open(os.path.join(PATH,"fixtures/long.bin"), 'r') as f:
            for l in f:
                t.write(l)
            t.close()
        # check encrypted file on disk did not change
        self.assertEquals('06345B901BF761E14EEA841D322CA2E1', 
            hashfile(os.path.join(PATH,"long.bin.crypted.trc")))

        # check round trip
        t2 = CryptoTrace(os.path.join(PATH,"long.bin.crypted.trc"), "r")
        with open(os.path.join(PATH,"long.bin.roundtrip"), 'w') as f:
            while True:
                data = t2.read(1024)
                if len(data) == 0:
                    break
                f.write(data)
            t2.close()

        # check decrypted file on disk same as original
        self.assertEquals('36209641806DB628D04F239192CD8E8C', 
            hashfile(os.path.join(PATH,"long.bin.roundtrip")))


    def test_flattextfile(self):
        t = open(os.path.join(PATH,"text.txt.clear.trc"), "w")
        with open(os.path.join(PATH,"fixtures/text.txt"), 'r') as f:
            for l in f:
                t.write(l)
            t.close()
        # check encrypted file on disk did not change
        self.assertEquals('1F439C7547B1EF1C1D3085483A87AD2E',
             hashfile(os.path.join(PATH,"text.txt.clear.trc")))

        # check round trip
        t2 = Trace(os.path.join(PATH,"text.txt.clear.trc"))
        with open(os.path.join(PATH,"text.txt.clear.roundtrip"), 'w') as f:
            while True:
                data = t2.read(1024)
                if len(data) == 0:
                    break
                f.write(data)

        # check decrypted file on disk same as original
        self.assertEquals('1F439C7547B1EF1C1D3085483A87AD2E',
            hashfile(os.path.join(PATH,"text.txt.clear.roundtrip")))

    def test_flatlongfile(self):
        t = open(os.path.join(PATH,"long.bin.clear.trc"), "w")
        with open(os.path.join(PATH,"fixtures/long.bin"), 'r') as f:
            for l in f:
                t.write(l)
            t.close()
        # check encrypted file on disk did not change
        self.assertEquals('36209641806DB628D04F239192CD8E8C',
            hashfile(os.path.join(PATH,"long.bin.clear.trc")))

        # check round trip
        t2 = Trace(os.path.join(PATH,"long.bin.clear.trc"))
        with open(os.path.join(PATH,"long.bin.clear.roundtrip"), 'w') as f:
            while True:
                data = t2.read(1024)
                if len(data) == 0:
                    break
                f.write(data)

        # check decrypted file on disk same as original
        self.assertEquals('36209641806DB628D04F239192CD8E8C', 
            hashfile(os.path.join(PATH,"long.bin.clear.roundtrip")))

if __name__ == '__main__':
    unittest.main()

