#!/usr/bin/python -O

import unittest
import os

from redcryptofile.encrypter import CryptoWriter
from redcryptofile.decrypter import CryptoReader

class TestEncrypter(unittest.TestCase):

    def test_writer2(self):
        checksums = []
        test_file = "/tmp/clear2.txt"
        with CryptoWriter(0, 0, test_file, checksums, random_type="LCG") as x:
            to_send = b"We write, and again, and so on."
            while to_send:
                res = x.write(to_send)
                to_send = to_send[res:]
            self.assertEqual(len(to_send), 0)
        os.unlink(test_file)

    def test_writer_checksum2(self):
        checksums = []
        test_file = "/tmp/clear.txt"
        with CryptoWriter(0, 1, test_file, checksums, random_type="LCG") as x:
            to_send = b"We write, and again, and so on."
            while to_send:
                res = x.write(to_send)
                to_send = to_send[res:]
            self.assertEqual(len(to_send), 0)
        os.unlink(test_file)

        self.assertEqual(checksums,
            ['C528B474843D8B14CF5BF43A9C049AF3239FAC564D86B4329069B5E145D0769B',
             'C528B474843D8B14CF5BF43A9C049AF3239FAC564D86B4329069B5E145D0769B'])

    def test_writer_encryption2(self):
        checksums = []
        test_file = "/tmp/encrypted.txt"
        with CryptoWriter(1, 1, test_file, checksums, random_type="LCG") as x:
            to_send = b"We write, and again, and so on."
            while to_send:
                res = x.write(to_send)
                to_send = to_send[res:]
            self.assertEqual(len(to_send), 0)
        os.unlink(test_file)

        self.assertEqual(checksums,
            ['2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5',
             '2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5'])

    def test_reader_clear(self):
        test_text = b"We write, and \0again, and so on."
        test_file = "./clear.txt"

        checksums = []
        with CryptoWriter(0, 0, test_file, checksums, random_type="LCG") as x:
            to_send = test_text
            while to_send:
                res = x.write(to_send)
                to_send = to_send[res:]
            self.assertEqual(len(to_send), 0)

        data = ""
        with CryptoReader(test_file) as cr:
            data = "".join(line for line in cr.read())
        os.unlink(test_file)

        self.assertEqual(data, test_text)


    def test_reader_checksum(self):
        test_text = b"We write, and \0again, and so on."
        test_file = "./check.txt"

        checksums = []
        with CryptoWriter(0, 1, test_file, checksums, random_type="LCG") as x:
            to_send = test_text
            while to_send:
                res = x.write(to_send)
                to_send = to_send[res:]
            self.assertEqual(len(to_send), 0)

        self.assertEqual(checksums,
            ['788A99BDCFFC5128DA01B9A6AABA9D826DCE7B874153417097EF2A82CF7875C6',
            '788A99BDCFFC5128DA01B9A6AABA9D826DCE7B874153417097EF2A82CF7875C6'])

        data = ""
        with CryptoReader(test_file) as cr:
            data = "".join(line for line in cr.read())
            self.assertEqual(data, test_text)
            checksums = cr.hash()
        os.unlink(test_file)

        self.assertEqual(checksums,
            ['788A99BDCFFC5128DA01B9A6AABA9D826DCE7B874153417097EF2A82CF7875C6',
            '788A99BDCFFC5128DA01B9A6AABA9D826DCE7B874153417097EF2A82CF7875C6'])

    def test_reader_largefile(self):
        buffersize = 65536
        checksums = []
        original_file = "./tests/includes/fixtures/dump_TLSw2008.hpp"
        test_file = "/tmp/dump.enc"
        with open(original_file, "r") as f:
            with CryptoWriter(1, 1, test_file, checksums, random_type="LCG") as fw:
                total_len = 0
                total_write_len = 0
                while True:
                    chunk = f.read(40000)
                    #print("read chunk %d:%d:%d" % (total_len, total_len + len(chunk),len(chunk)))
                    total_len += len(chunk)
                    if not chunk:
                        break
                    ret = fw.write(chunk)
                    #print("write chunk %d:%d" % (total_write_len, ret))
                    total_write_len += ret
                    chunk = None


        self.assertEqual(checksums,
            ['77770CC3C347D5DDC0139ED6C6F794C4911FFD52348B26981812353C961A6456',
            'D2E4E91CB003C5F56344E2BAE1535AD9922E0C1E667898D443A21282E14F1C05'])

        data = ""
        total = 0
        with CryptoReader(test_file) as cr:
            for line in cr.read(buffersize):
                #print("line read=%d total=%d" % (len(line), total))
                data += line
                total += len(line)

        self.assertEqual(checksums,
            ['77770CC3C347D5DDC0139ED6C6F794C4911FFD52348B26981812353C961A6456',
             'D2E4E91CB003C5F56344E2BAE1535AD9922E0C1E667898D443A21282E14F1C05'])

        #with open("/tmp/dump.txt", "w") as f:
            #f.write(data)

        content = ""
        with open(original_file, 'r') as content_file:
            content = "".join(line for line in content_file.read())
        self.assertEqual(data, content)

        os.unlink(test_file)

if __name__ == '__main__':
    unittest.main()

