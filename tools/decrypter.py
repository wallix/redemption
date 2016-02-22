#!/usr/bin/python -O

import ctypes
import ctypes.util

GETHMACKEY = ctypes.CFUNCTYPE(ctypes.c_char_p)
GETTRACEKEY = ctypes.CFUNCTYPE(ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int)

def get_hmac_key():
    return "000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F"

def get_trace_key(base, lg):
    return "000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F"


get_hmac_key_func = GETHMACKEY(get_hmac_key)
get_trace_key_func = GETTRACEKEY(get_trace_key)

try:
    libredver = ctypes.CDLL('/usr/lib/libredver.so')
    libredver.do_main.argtypes = [ctypes.c_uint, ctypes.POINTER(ctypes.c_char_p), GETHMACKEY, GETTRACEKEY]
    libredver.do_main.restype = ctypes.c_int

    import sys
    myargv = ctypes.c_char_p * len(sys.argv)
    argv = myargv(*tuple(sys.argv))
    libreddec_main(len(sys.argv), argv, get_hmac_key_func, get_trace_key_func)
except Exception, e:
    import traceback
    print "Failed to load redver library\n", traceback.format_exc(e)
