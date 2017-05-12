#!/usr/bin/python -O

import sys
import ctypes
import ctypes.util

from ctypes import CFUNCTYPE, c_ulong, c_ulonglong, c_int, c_char_p, c_uint16, py_object, c_uint, c_uint32, c_uint64, c_float, c_void_p, POINTER, Structure

pathlib = 'libredcryptofile.so'

from redcryptofile.keys import GETHMACKEY, GETTRACEKEY, get_hmac_key_func, get_trace_key_func

from redcryptofile import lib
    
        
    
import unittest
    
class TestEncrypter(unittest.TestCase):

    def test_writer2(self):
        from redcryptofile.encrypter import CryptoWriter
        with CryptoWriter(0, 0, "./xxx.txt") as x:
            x.write("this is my test", 15)

    def test_writer(self):
        handle = lib.redcryptofile_writer_new(0, 0, get_hmac_key_func, get_trace_key_func)
        self.assertNotEqual(handle, None)
        lib.redcryptofile_writer_open(handle, "./clear.txt")
        text = b"We write, and again, and so on."
        total_sent = 0
        
        while total_sent < len(text):
            part_len = min(10,len(text[total_sent:]))
            res = lib.redcryptofile_writer_write(handle, text[total_sent:], part_len)
            self.assertTrue(res > 0)
            if res < 0: 
                break
            total_sent += res
        
        self.assertEqual(total_sent, 31)
        lib.redcryptofile_writer_close(handle)
        lib.redcryptofile_writer_delete(handle)


    def test_writer_checksum(self):
        handle = lib.redcryptofile_writer_new(0, 1, get_hmac_key_func, get_trace_key_func)
        self.assertNotEqual(handle, None)
        lib.redcryptofile_writer_open(handle, "./clear.txt")
        text = b"We write, and again, and so on."
        total_sent = 0
        
        while total_sent < len(text):
            part_len = min(10,len(text[total_sent:]))
            res = lib.redcryptofile_writer_write(handle, text[total_sent:], part_len)
            self.assertTrue(res > 0)
            if res < 0: 
                break
            total_sent += res
        
        self.assertEqual(total_sent, 31)
        lib.redcryptofile_writer_close(handle)

        self.assertEqual(lib.redcryptofile_writer_qhashhex(handle),         
            'C528B474843D8B14CF5BF43A9C049AF3239FAC564D86B4329069B5E145D0769B')

        self.assertEqual(lib.redcryptofile_writer_fhashhex(handle),         
            'C528B474843D8B14CF5BF43A9C049AF3239FAC564D86B4329069B5E145D0769B')

        lib.redcryptofile_writer_delete(handle)

    def test_writer_encryption_checksum(self):
        handle = lib.redcryptofile_writer_new(1, 1, get_hmac_key_func, get_trace_key_func)
        self.assertNotEqual(handle, None)
        lib.redcryptofile_writer_open(handle, "./encrypted.txt")
        text = b"We write, and again, and so on."
        total_sent = 0
        
        while total_sent < len(text):
            part_len = min(10,len(text[total_sent:]))
            res = lib.redcryptofile_writer_write(handle, text[total_sent:], part_len)
            self.assertTrue(res > 0)
            if res < 0: 
                break
            total_sent += res
        
        self.assertEqual(total_sent, 31)
        lib.redcryptofile_writer_close(handle)

        self.assertEqual(lib.redcryptofile_writer_qhashhex(handle),
            '2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5')

        self.assertEqual(lib.redcryptofile_writer_fhashhex(handle),         
            '2ACC1E2CBFFE64030D50EAE7845A9DCE6EC4E84AC2435F6C0F7F16F87B0180F5')

        lib.redcryptofile_writer_delete(handle)

    def test_reader_clear(self):
        handle_w = lib.redcryptofile_writer_new(0, 0, get_hmac_key_func, get_trace_key_func)
        self.assertNotEqual(handle_w, None)
        lib.redcryptofile_writer_open(handle_w, "./clear.txt")
        text = b"We write, and again, and so on."
        total_sent = 0
        
        while total_sent < len(text):
            part_len = min(10,len(text[total_sent:]))
            res = lib.redcryptofile_writer_write(handle_w, text[total_sent:], part_len)
            self.assertTrue(res > 0)
            if res < 0: 
                break
            total_sent += res
        self.assertEqual(total_sent, 31)

        handle_r = lib.redcryptofile_reader_new(get_hmac_key_func, get_trace_key_func)
        self.assertNotEqual(handle_r, None)
        lib.redcryptofile_reader_open(handle_r, "./clear.txt");
        data = ctypes.create_string_buffer(31)
        total_read = 0

        while total_read < 31:
            part_len = min(10,len(data[total_read:]))
            buf = ctypes.create_string_buffer(10)
            res = lib.redcryptofile_reader_read(handle_r, buf, part_len)
            self.assertTrue(res > 0)
            if res < 0: 
                break
            data[total_read:total_read + part_len] = ''.join(buf.value)
            total_read += res

        self.assertEqual(total_read, 31)
        self.assertEqual(data.value, "We write, and again, and so on.")
        lib.redcryptofile_reader_close(handle_r)
        lib.redcryptofile_reader_delete(handle_r)
        lib.redcryptofile_writer_close(handle_w)
        lib.redcryptofile_writer_delete(handle_w)

    def test_reader_checksum(self):
        handle_w = lib.redcryptofile_writer_new(0, 1, get_hmac_key_func, get_trace_key_func)
        self.assertNotEqual(handle_w, None)
        lib.redcryptofile_writer_open(handle_w, "./clear.txt")
        text = b"We write, and again, and so on."
        total_sent = 0
        
        while total_sent < len(text):
            part_len = min(10,len(text[total_sent:]))
            res = lib.redcryptofile_writer_write(handle_w, text[total_sent:], part_len)
            self.assertTrue(res > 0)
            if res < 0: 
                break
            total_sent += res
        self.assertEqual(total_sent, 31)

        handle_r = lib.redcryptofile_reader_new(get_hmac_key_func, get_trace_key_func)
        self.assertNotEqual(handle_r, None)
        lib.redcryptofile_reader_open(handle_r, "./clear.txt");
        data = ctypes.create_string_buffer(31)
        total_read = 0

        while total_read < 31:
            part_len = min(10,len(data[total_read:]))
            buf = ctypes.create_string_buffer(10)
            res = lib.redcryptofile_reader_read(handle_r, buf, part_len)
            self.assertTrue(res > 0)
            if res < 0: 
                break
            data[total_read:total_read + part_len] = ''.join(buf.value)
            total_read += res

        self.assertEqual(total_read, 31)
        self.assertEqual(data.value, "We write, and again, and so on.")
        lib.redcryptofile_reader_close(handle_r)
        lib.redcryptofile_reader_delete(handle_r)
        lib.redcryptofile_writer_close(handle_w)
        lib.redcryptofile_writer_delete(handle_w)


    def test_reader_encrypted(self):
        handle_w = lib.redcryptofile_writer_new(1, 1, get_hmac_key_func, get_trace_key_func)
        self.assertNotEqual(handle_w, None)
        lib.redcryptofile_writer_open(handle_w, "./encrypted.txt")
        text = b"We write, and again, and so on."
        total_sent = 0
        
        while total_sent < len(text):
            part_len = min(10,len(text[total_sent:]))
            res = lib.redcryptofile_writer_write(handle_w, text[total_sent:], part_len)
            self.assertTrue(res > 0)
            if res < 0: 
                break
            total_sent += res
        self.assertEqual(total_sent, 31)

        handle_r = lib.redcryptofile_reader_new(get_hmac_key_func, get_trace_key_func)
        self.assertNotEqual(handle_r, None)
        lib.redcryptofile_reader_open(handle_r, "./encrypted.txt");
        data = ctypes.create_string_buffer(31)
        total_read = 0

        while total_read < 31:
            part_len = min(10,len(data[total_read:]))
            buf = ctypes.create_string_buffer(10)
            res = lib.redcryptofile_reader_read(handle_r, buf, part_len)
            self.assertTrue(res > 0)
            if res < 0: 
                break
            data[total_read:total_read + part_len] = ''.join(buf.value)
            total_read += res

        self.assertEqual(total_read, 31)
        self.assertEqual(data.value, "We write, and again, and so on.")
        lib.redcryptofile_reader_close(handle_r)
        lib.redcryptofile_reader_delete(handle_r)
        lib.redcryptofile_writer_close(handle_w)
        lib.redcryptofile_writer_delete(handle_w)

if __name__ == '__main__':
    unittest.main()
        
