# Do not remove

import sys
import ctypes
import ctypes.util

from ctypes import CFUNCTYPE, c_ulong, c_ulonglong, c_int, c_char_p, c_uint16, py_object, c_uint, c_uint32, c_uint64, c_float, c_void_p, POINTER, Structure

pathlib = 'libscytale.so'
import keys

try:
    lib = ctypes.CDLL(pathlib)
    print("load {path} OK".format(path=pathlib))

    # char const * scytale_version()
    lib.scytale_version.argtypes = []
    lib.scytale_version.restype = c_char_p

# RedCryptoWriterHandle * scytale_writer_new(
#    int with_encryption, int with_checksum, get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn, int old_scheme, int one_shot)
    lib.scytale_writer_new.argtypes = [ c_int, c_int, keys.GETHMACKEY, keys.GETTRACEKEY, c_int, c_int ]
    lib.scytale_writer_new.restype = c_void_p

# RedCryptoWriterHandle * scytale_writer_new_with_test_random(
#    int with_encryption, int with_checksum, get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn, int old_scheme, int one_shot)
    lib.scytale_writer_new_with_test_random.argtypes = [ c_int, c_int, keys.GETHMACKEY, keys.GETTRACEKEY, c_int, c_int ]
    lib.scytale_writer_new_with_test_random.restype = c_void_p

# int scytale_writer_open(RedCryptoWriterHandle * handle, char const * path, char const * hashpath, int groupid)
    lib.scytale_writer_open.argtypes = [ c_void_p, c_char_p, c_char_p, c_int ]
    lib.scytale_writer_open.restype = c_int

# int scytale_writer_write(RedCryptoWriterHandle * handle, uint8_t const * buffer, unsigned long len);
    lib.scytale_writer_write.argtypes = [c_void_p, c_char_p, c_uint64 ]
    lib.scytale_writer_write.restype = c_int

# int scytale_writer_close(RedCryptoWriterHandle * handle);
    lib.scytale_writer_close.argtypes = [ c_void_p ]
    lib.scytale_writer_close.restype = c_int

# void scytale_writer_delete(RedCryptoWriterHandle * handle);
    lib.scytale_writer_delete.argtypes = [ c_void_p ]
    lib.scytale_writer_delete.restype = None

# const char * scytale_writer_qhashhex(RedCryptoWriterHandle * handle);
    lib.scytale_writer_qhashhex.argtypes = [ c_void_p ]
    lib.scytale_writer_qhashhex.restype = c_char_p

# const char * scytale_writer_fhashhex(RedCryptoWriterHandle * handle);
    lib.scytale_writer_fhashhex.argtypes = [ c_void_p ]
    lib.scytale_writer_fhashhex.restype = c_char_p

# RedCryptoReaderHandle * scytale_reader_new(const char * derivator, get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn, int old_scheme, int one_shot)
    lib.scytale_reader_new.argtypes = [ c_char_p, keys.GETHMACKEY, keys.GETTRACEKEY, c_int, c_int ]
    lib.scytale_reader_new.restype = c_void_p

# char const * scytale_reader_error_message(RedCryptoReaderHandle * handle);
    lib.scytale_reader_error_message.argtypes = [c_void_p]
    lib.scytale_reader_error_message.restype = c_char_p

# int scytale_reader_open(RedCryptoReaderHandle * handle, char const * path, char const * derivator)
    lib.scytale_reader_open.argtypes = [ c_void_p, c_char_p, c_char_p ]
    lib.scytale_reader_open.restype = c_int

# int scytale_reader_read(RedCryptoReaderHandle * handle, uint8_t const * buffer, unsigned long len);
    lib.scytale_reader_read.argtypes = [c_void_p, c_char_p, c_uint64 ]
    lib.scytale_reader_read.restype = c_int

# int scytale_reader_close(RedCryptoReaderHandle * handle);
    lib.scytale_reader_close.argtypes = [ c_void_p ]
    lib.scytale_reader_close.restype = c_int

# void scytale_reader_delete(RedCryptoReaderHandle * handle);
    lib.scytale_reader_delete.argtypes = [ c_void_p ]
    lib.scytale_reader_delete.restype = None

# int scytale_reader_fhash(RedCryptoReaderHandle * handle, const char * file);
    lib.scytale_reader_fhash.argtypes = [ c_void_p, c_char_p ]
    lib.scytale_reader_fhash.restype = c_int

# int scytale_reader_qhash(RedCryptoReaderHandle * handle, const char * file);
    lib.scytale_reader_qhash.argtypes = [ c_void_p, c_char_p ]
    lib.scytale_reader_qhash.restype = c_int

# const char * scytale_qhashhex(RedCryptoReaderHandle * handle);
    lib.scytale_reader_qhashhex.argtypes = [ c_void_p ]
    lib.scytale_reader_qhashhex.restype = c_char_p

# const char * scytale_fhashhex(RedCryptoReaderHandle * handle);
    lib.scytale_reader_fhashhex.argtypes = [ c_void_p ]
    lib.scytale_reader_fhashhex.restype = c_char_p

except Exception as e:
    print("Failed to load scytale library: %s\n" % str(e))
    sys.exit(10)

from encrypter import CryptoWriter
encrypter.lib = lib
encrypter.get_hmac_key_func = keys.get_hmac_key_func
encrypter.get_trace_key_func = keys.get_trace_key_func

from decrypter import CryptoReader
decrypter.lib = lib
decrypter.get_hmac_key_func = keys.get_hmac_key_func
decrypter.get_trace_key_func = keys.get_trace_key_func
