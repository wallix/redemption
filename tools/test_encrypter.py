#!/usr/bin/python -O
  
import unittest

from redcryptofile.encrypter import CryptoWriter
from redcryptofile.decrypter import CryptoReader

class TestEncrypter(unittest.TestCase):

#    def test_writer2(self):
#        try:
#            checksums = []
#            with CryptoWriter(0, 0, "./tmp/clear2.txt", checksums) as x:
#                to_send = b"We write, and again, and so on."
#                while to_send:
#                    res = x.write(to_send)
#                    to_send = to_send[res:]
#                self.assertEqual(len(to_send), 0)
#        except:
#            self.assertTrue(False)

#    def test_writer_checksum2(self):
#        try:
#            checksums = []
#            with CryptoWriter(0, 1, "./clear.txt", checksums) as x:
#                to_send = b"We write, and again, and so on."
#                while to_send:
#                    res = x.write(to_send)
#                    to_send = to_send[res:]
#                self.assertEqual(len(to_send), 0)
#        except:
#            self.assertTrue(False)

#        self.assertEqual(checksums,
#            ['C528B474843D8B14CF5BF43A9C049AF3239FAC564D86B4329069B5E145D0769B',
#             'C528B474843D8B14CF5BF43A9C049AF3239FAC564D86B4329069B5E145D0769B'])

#    def test_writer_encryption2(self):
#        try:
#            checksums = []
#            with CryptoWriter(1, 1, "./encrypted.txt", checksums) as x:
#                to_send = b"We write, and again, and so on."
#                while to_send:
#                    res = x.write(to_send)
#                    to_send = to_send[res:]
#                self.assertEqual(len(to_send), 0)
#        except:
#            self.assertTrue(False)

#        self.assertEqual(checksums,
#            ['2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5',
#             '2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5'])

#    def test_reader_clear(self):
#    
#        test_text = b"We write, and \0again, and so on."
#        test_file = "./clear.txt"
#        
#        checksums = []
#        with CryptoWriter(0, 0, test_file, checksums) as x:
#            to_send = test_text
#            while to_send:
#                res = x.write(to_send)
#                to_send = to_send[res:]
#            self.assertEqual(len(to_send), 0)

#        data = ""
#        with CryptoReader(test_file) as cr:
#            data = "".join(line for line in cr.read())
#        self.assertEqual(data, test_text)


#    def test_reader_checksum(self):
#        test_text = b"We write, and \0again, and so on."
#        test_file = "./check.txt"
#        
#        checksums = []
#        with CryptoWriter(0, 1, test_file, checksums) as x:
#            to_send = test_text
#            while to_send:
#                res = x.write(to_send)
#                to_send = to_send[res:]
#            self.assertEqual(len(to_send), 0)

#        self.assertEqual(checksums,
#         ['788A99BDCFFC5128DA01B9A6AABA9D826DCE7B874153417097EF2A82CF7875C6',
#          '788A99BDCFFC5128DA01B9A6AABA9D826DCE7B874153417097EF2A82CF7875C6'])

#        data = ""
#        with CryptoReader(test_file) as cr:
#            data = "".join(line for line in cr.read())
#            self.assertEqual(data, test_text)
#            checksums = cr.hash()

#        self.assertEqual(checksums,
#         ['788A99BDCFFC5128DA01B9A6AABA9D826DCE7B874153417097EF2A82CF7875C6',
#          '788A99BDCFFC5128DA01B9A6AABA9D826DCE7B874153417097EF2A82CF7875C6'])

    def test_reader_largefile(self):

        buffersize = 20000
        checksums = []    
        with open("/home/cgrosjean/d/ssh/Fargo.avi", "r") as f:
            with CryptoWriter(1, 1, "./newfar4.avi", checksums) as fw:
                total_len = 0
                total_write_len = 0
                while True:
                    chunk = f.read(buffersize)
                    print("read chunk %d:%d:%d" % (total_len, total_len + len(chunk),len(chunk)))
                    total_len += len(chunk)
                    if not chunk:
                        break
                    ret = fw.write(chunk)
                    print("write chunk %d:%d" % (total_write_len, ret))
                    chunk = None


#        self.assertEqual(checksums,
#         ['C9A7B84600520B3365E2E71248D29873D446FC2DE2303305D0BCA955FB672036',
#          '0D90D013E1F93279F3B295BC5D67227A816C1E7BFB4987E9FC6DCC2BF363AFB9'])

#        print "Done writing newfar"
    
        data = ""
        total = 0
        with open("./Fargo2.avi", "w") as f:
            with CryptoReader("./newfar4.avi") as cr:
                for line in cr.read(16384):
                    print("line read=%d total=%d" % (len(line), total))
                    total += len(line)
                    f.write(line)
#                checksums = cr.hash()

#        self.assertEqual(checksums,
#         ['788A99BDCFFC5128DA01B9A6AABA9D826DCE7B874153417097EF2A82CF7875C6',
#          '788A99BDCFFC5128DA01B9A6AABA9D826DCE7B874153417097EF2A82CF7875C6'])


if __name__ == '__main__':
    unittest.main()
        
