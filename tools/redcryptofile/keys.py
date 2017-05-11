import ctypes
import ctypes.util

from ctypes import CFUNCTYPE, c_int, c_uint, c_char_p, c_void_p


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

