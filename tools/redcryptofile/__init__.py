# Do not remove

import sys
import ctypes
import ctypes.util

from ctypes import CFUNCTYPE, c_ulong, c_ulonglong, c_int, c_char_p, c_uint16, py_object, c_uint, c_uint32, c_uint64, c_float, c_void_p, POINTER, Structure

pathlib = 'libredcryptofile.so'
import keys

try:
    lib = ctypes.CDLL(pathlib)
    print("load {path} OK".format(path=pathlib))

# RedCryptoWriterHandle * redcryptofile_writer_new(
#    int with_encryption, int with_checksum, get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn)
    lib.redcryptofile_writer_new.argtypes = [ c_int, c_int, keys.GETHMACKEY, keys.GETTRACEKEY ]
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
    lib.redcryptofile_reader_new.argtypes = [ keys.GETHMACKEY, keys.GETTRACEKEY ]
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

# int redcryptofile_reader_hash(RedCryptoReaderHandle * handle, const char * file);
    lib.redcryptofile_reader_hash.argtypes = [ c_void_p, c_char_p ]
    lib.redcryptofile_reader_hash.restype = c_int

# const char * redcryptofile_qhashhex(RedCryptoReaderHandle * handle);
    lib.redcryptofile_reader_qhashhex.argtypes = [ c_void_p ]
    lib.redcryptofile_reader_qhashhex.restype = c_char_p
    
# const char * redcryptofile_fhashhex(RedCryptoReaderHandle * handle);
    lib.redcryptofile_reader_fhashhex.argtypes = [ c_void_p ]
    lib.redcryptofile_reader_fhashhex.restype = c_char_p 

except Exception as e:
    print("Failed to load redcryptofile library: %s\n" % str(e))
    sys.exit(10)
    
from encrypter import CryptoWriter
encrypter.lib = lib
encrypter.get_hmac_key_func = keys.get_hmac_key_func
encrypter.get_trace_key_func = keys.get_trace_key_func

from decrypter import CryptoReader
decrypter.lib = lib
decrypter.get_hmac_key_func = keys.get_hmac_key_func
decrypter.get_trace_key_func = keys.get_trace_key_func
