#!/usr/bin/python -O

import ctypes
import ctypes.util

GETKEY = ctypes.CFUNCTYPE(ctypes.c_char_p)

def get_key():
    return "000102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F"

get_key_func = GETKEY(get_key)

try:
#    libpath = ctypes.util.find_library('libredver')
    libredver = ctypes.CDLL('/usr/lib/libredver.so')

    # int do_main(int argc, char ** argv)
    libredver.do_main.argtypes = [ctypes.c_uint, ctypes.POINTER(ctypes.c_char_p), GETKEY]
    libredver.do_main.restype = ctypes.c_int

    #myargv = ctypes.c_char_p * 2
    #argv = myargv("One", "Two")
    #argc = ctypes.c_int ( 2 )

    import sys
    myargv = ctypes.c_char_p * len(sys.argv)
    argv = myargv(*tuple(sys.argv))
    print "Calling do_main\n";
    libredver.do_main(len(sys.argv), argv, get_key_func)
    print "Returned from do_main\n";
except Exception, e:
    import traceback
    print "Failed to load redver library\n", traceback.format_exc(e)
