#!/usr/bin/python -O

import ctypes
import ctypes.util

GETHMACKEY = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_void_p)
GETTRACEKEY = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_char_p, ctypes.c_int, ctypes.c_void_p)

def get_hmac_key(resbuf):
    data = '\xe3\x8d\xa1\x5e\x50\x1e\x4f\x6a\x01\xef\xde\x6c\xd9\xb3\x3a\x3f\x2b\x41\x72\x13\x1e\x97\x5b\x4c\x39\x54\x23\x14\x43\xae\x22\xae'
    libredver.vermemcpy(c_void_p, data, 32)
    return 0

def get_trace_key(base, lg, resbuf):
    name = base[:lg]
    data = '\x56\x3e\xb6\xe8\x15\x8f\x0e\xed\x2e\x5f\xb6\xbc\x28\x93\xbc\x15\x27\x0d\x7e\x78\x15\xfa\x80\x4a\x72\x3e\xf4\xfb\x31\x5f\xf4\xb2'
#    data = '\x47\xa0\xb0\xe8\xc3\x86\x72\xb9\x66\x25\xf2\x35\x47\xa8\xc5\xab\x8c\x7c\xf2\x5b\x88\xc0\x23\xb0\xac\x03\xaa\x5d\x5b\xca\xf9\xda'
    libredver.vermemcpy(resbuf, data, 32)
    return 0
    

get_hmac_key_func = GETHMACKEY(get_hmac_key)
get_trace_key_func = GETTRACEKEY(get_trace_key)

try:
    libredver = ctypes.CDLL('/usr/lib/libredver.so')
    libredver.do_main.argtypes = [ctypes.c_uint, ctypes.POINTER(ctypes.c_char_p), GETHMACKEY, GETTRACEKEY]
    libredver.do_main.restype = ctypes.c_int

    libredver.vermemcpy.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
    libredver.vermemcpy.restype = ctypes.c_int

    import sys
    myargv = ctypes.c_char_p * len(sys.argv)
    argv = myargv(*tuple(sys.argv))
    libredver.do_main(len(sys.argv), argv, get_hmac_key_func, get_trace_key_func)
except Exception, e:
    import traceback
    print "Failed to load redver library\n", traceback.format_exc(e)
