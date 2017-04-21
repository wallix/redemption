#!/usr/bin/python -O

import sys
import ctypes
import ctypes.util

pathlib = '/usr/local/lib/libredcryptofile.so'

hmac_key_str = '\xe3\x8d\xa1\x5e\x50\x1e\x4f\x6a\x01\xef\xde\x6c\xd9\xb3\x3a\x3f\x2b\x41\x72\x13\x1e\x97\x5b\x4c\x39\x54\x23\x14\x43\xae\x22\xae'

key_str = '\x56\x3e\xb6\xe8\x15\x8f\x0e\xed\x2e\x5f\xb6\xbc\x28\x93\xbc\x15\x27\x0d\x7e\x78\x15\xfa\x80\x4a\x72\x3e\xf4\xfb\x31\x5f\xf4\xb2'

GETHMACKEY = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_void_p)
GETTRACEKEY = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_char_p, ctypes.c_int, ctypes.c_void_p)

def get_hmac_key(resbuf):
    ctypes.memmove(resbuf, hmac_key_str, 32)
    return 0

def get_trace_key(base, lg, resbuf, flag):
    name = base[:lg]
    ctypes.memmove(resbuf, key_str, 32)
    return 0

get_hmac_key_func = GETHMACKEY(get_hmac_key)
get_trace_key_func = GETTRACEKEY(get_trace_key)

# RedCryptoWriterHandle * redcryptofile_open_writer(
#    int with_encryption, int with_checksum, const char * path,
#    get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn)

libredrec.redcryptofile_open_writer.argtypes = [ctypes.c_int, ctypes.c_int, 
                                                ctypes.POINTER(ctypes.c_char_p), 
                                                GETHMACKEY, GETTRACEKEY]
libredrec.redcryptofile_open_writer.restype = ctypes.c_void_p


try:
    libredcryptofile = ctypes.CDLL(pathlib)
    print("load {path} OK".format(path=pathlib))
except Exception as e:
    print("Failed to load redcryptofile library: %s\n" % str(e))
    sys.exit(10)
