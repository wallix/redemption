#!/usr/bin/python -O

import sys
import ctypes
import ctypes.util

from ctypes import CFUNCTYPE, c_ulong, c_ulonglong, c_int, c_char_p, c_uint16, py_object, c_uint, c_uint32, c_uint64, c_float, c_void_p, POINTER, Structure

#pathlib = '/usr/local/lib/libredcryptofile.so'
#pathlib = './bin/clang-linux-3.8.1/debug/libredcryptofile.so'
pathlib = './bin/gcc-4.9.2/release/libredcryptofile.so'

hmac_key_str = '\x86\x41\x05\x58\xc4\x95\xcc\x4e\x49\x21\x57\x87\x47\x74\x08\x8a\x33\xb0\x2a\xb8\x65\xcc\x38\x41\x20\xfe\xc2\xc9\xb8\x72\xc8\x2c'
key_str = '\x61\x1f\xd4\xcd\xe5\x95\xb7\xfd\xa6\x50\x38\xfc\xd8\x86\x51\x4f\x59\x7e\x8e\x90\x81\xf6\xf4\x48\x9c\x77\x41\x51\x0f\x53\x0e\xe8'

GETHMACKEY = CFUNCTYPE(c_int, c_void_p)
GETTRACEKEY = CFUNCTYPE(c_int, c_char_p, c_int, c_void_p, c_uint)

def get_hmac_key(resbuf):
    ctypes.memmove(resbuf, hmac_key_str, 32)
    return 0

def get_trace_key(base, lg, resbuf, flag):
    name = base[:lg]
    ctypes.memmove(resbuf, key_str, 32)
    return 0

get_hmac_key_func = GETHMACKEY(get_hmac_key)
get_trace_key_func = GETTRACEKEY(get_trace_key)

try:
    lib = ctypes.CDLL(pathlib)
    print("load {path} OK".format(path=pathlib))

# RedCryptoWriterHandle * redcryptofile_writer_new(
#    int with_encryption, int with_checksum, get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn)
    lib.redcryptofile_writer_new.argtypes = [ c_int, c_int, GETHMACKEY, GETTRACEKEY ]
    lib.redcryptofile_writer_new.restype = c_void_p

# int redcryptofile_writer_open(RedCryptoWriterHandle * handle, const char * path)
    lib.redcryptofile_writer_open.argtypes = [ c_void_p, c_char_p ]
    lib.redcryptofile_writer_open.restype = c_int

# int redcryptofile_writer_write(RedCryptoWriterHandle * handle, uint8_t const * buffer, unsigned long len);
    lib.redcryptofile_writer_write.argtypes = [c_void_p, c_char_p, c_uint64 ]
    lib.redcryptofile_writer_write.restype = c_int

# int redcryptofile_writer_close(RedCryptoWriterHandle * handle);
    lib.redcryptofile_writer_close.argtypes = [ c_void_p ]
    lib.redcryptofile_writer_close.restype = c_int

# void redcryptofile_writer_delete(RedCryptoWriterHandle * handle);
    lib.redcryptofile_writer_delete.argtypes = [ c_void_p ]
    lib.redcryptofile_writer_delete.restype = None

# const char * redcryptofile_writer_qhashhex(RedCryptoWriterHandle * handle);
    lib.redcryptofile_writer_qhashhex.argtypes = [ c_void_p ]
    lib.redcryptofile_writer_qhashhex.restype = c_char_p
    
# const char * redcryptofile_writer_fhashhex(RedCryptoWriterHandle * handle);
    lib.redcryptofile_writer_fhashhex.argtypes = [ c_void_p ]
    lib.redcryptofile_writer_fhashhex.restype = c_char_p



# RedCryptoReaderHandle * redcryptofile_reader_new(get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn)
    lib.redcryptofile_reader_new.argtypes = [ GETHMACKEY, GETTRACEKEY ]
    lib.redcryptofile_reader_new.restype = c_void_p

# int redcryptofile_reader_open(RedCryptoReaderHandle * handle, const char * path)
    lib.redcryptofile_reader_open.argtypes = [ c_void_p, c_char_p ]
    lib.redcryptofile_reader_open.restype = c_int

# int redcryptofile_reader_read(RedCryptoReaderHandle * handle, uint8_t const * buffer, unsigned long len);
    lib.redcryptofile_reader_read.argtypes = [c_void_p, c_char_p, c_uint64 ]
    lib.redcryptofile_reader_read.restype = c_int

# int redcryptofile_reader_close(RedCryptoReaderHandle * handle);
    lib.redcryptofile_reader_close.argtypes = [ c_void_p ]
    lib.redcryptofile_reader_close.restype = c_int

# void redcryptofile_reader_delete(RedCryptoReaderHandle * handle);
    lib.redcryptofile_reader_delete.argtypes = [ c_void_p ]
    lib.redcryptofile_reader_delete.restype = None   


# const char * redcryptofile_qhashhex(RedCryptoReaderHandle * handle);
##    lib.redcryptofile_reader_qhashhex.argtypes = [ c_void_p ]
##    lib.redcryptofile_reader_qhashhex.restype = c_char_p
    
# const char * redcryptofile_fhashhex(RedCryptoReaderHandle * handle);
##    lib.redcryptofile_reader_fhashhex.argtypes = [ c_void_p ]
##    lib.redcryptofile_reader_fhashhex.restype = c_char_p 


except Exception as e:
    print("Failed to load redcryptofile library: %s\n" % str(e))
    sys.exit(10)
    
    
#class CryptoWriter(Object):
#    def __init__(self, encryption, checksum, get_hmac_key_func, get_trace_key_func): 
#        self.handle = lib.redcryptofile_writer_new(0, 0, get_hmac_key_func, get_trace_key_func)
#    
#    def __del__(self):
#        lib.redcryptofile_writer_delete(self.handle)
#    
#    def __enter__(self):
#        lib.redcryptofile_writer_open(self.handle)

#    def __exit__(self, type, value, traceback):
#        lib.redcryptofile_writer_close(self.handle)
#        self.qhashhex = lib.redcryptofile_writer_qhashhex(self.handle).copy()
#        self.fhashhex = lib.redcryptofile_writer_fhashhex(self.handle).copy()
#        
#    def write(self, data, length):
#        return lib.redcryptofile_writer_write(self.handle, data, length)
        
    
import unittest
    
class TestEncrypter(unittest.TestCase):
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



    def test_reader(self):
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

        buf = ctypes.create_string_buffer(31)
        total_read = lib.redcryptofile_reader_read(handle_r, buf, 31)

        # TODO : Does not work for partial reading
        # while total_read < 31:
        #     part_len = min(10,len(buf[total_read:]))
        #     res = lib.redcryptofile_reader_read(handle_r, buf[total_read:], part_len)

        #     self.assertTrue(res > 0)
        #     if res < 0: 
        #         break
        #     total_read += res

        self.assertEqual(total_read, 31)
        print ">>>> %d" % total_read
        print ">>>> %s" % repr(buf.value)

        self.assertEqual(buf.value, "We write, and again, and so on.")

        # TODO : double free or corruption
        lib.redcryptofile_reader_close(handle_r)
        lib.redcryptofile_reader_delete(handle_r)

        lib.redcryptofile_writer_close(handle_w)
        lib.redcryptofile_writer_delete(handle_w)

if __name__ == '__main__':
    unittest.main()
        
