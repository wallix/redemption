#!/usr/bin/python -O
# -*- coding: utf-8 -*-
#

MK_SIZE = 32
MK_OFFSET = 40


from wabconfig import Config as cfg
from wabengine.client.sync_client import SynClient
from wabengine.common import exception as excp

from wabha.utils import *
import ctypes
import ctypes.util
import sys
import os
import pwd


def get_proxy():
    configs = cfg('wabengine')
    client = SynClient('localhost', configs['port'])
    try:
        proxy = client.authenticate_as_operator()
    except excp.AuthenticationFailed, exp:
        Logger().error("%s" % exp)
        sys.exit(1)
    return proxy


GETHMACKEY = ctypes.CFUNCTYPE(ctypes.c_int, 
                              ctypes.c_void_p)
                              
GETTRACEKEY = ctypes.CFUNCTYPE(ctypes.c_int, 
                               ctypes.c_char_p, ctypes.c_int, ctypes.c_void_p, ctypes.c_uint)


def get_hmac_key(resbuf):
    sign_key = proxy.get_trace_sign_key()
    libredrec.recmemcpy(resbuf, sign_key, 32)
    return 0


def get_trace_key(base, lg, resbuf, flag):
    name = base[:lg]
    trace_key = proxy.get_trace_encryption_key(name, flag==1)
    libredrec.recmemcpy(resbuf, trace_key, 32)
    return 0


get_hmac_key_func = GETHMACKEY(get_hmac_key)
get_trace_key_func = GETTRACEKEY(get_trace_key)

if __name__ == '__main__':
    try:
        original_gid = os.getegid()
        original_uid = os.geteuid()

        os.setegid(pwd.getpwnam("wabuser").pw_gid)
        os.seteuid(pwd.getpwnam("wabuser").pw_uid)
        if ha_status() > HA_MASTER:
            # If HA is configured we run this script only on the MASTER node
            sys.exit(1)
        proxy = get_proxy()

        os.seteuid(original_uid)
        os.setegid(original_gid)

        libredrec = ctypes.CDLL('/usr/lib/x86_64-linux-gnu/libredrec.so')
        libredrec.do_main.argtypes = [ctypes.c_uint, ctypes.POINTER(ctypes.c_char_p), GETHMACKEY, GETTRACEKEY]
        libredrec.do_main.restype = ctypes.c_int

        libredrec.recmemcpy.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        libredrec.recmemcpy.restype = ctypes.c_int

        argv = ["redrec"].extend(sys.argv)

        myargv = ctypes.c_char_p * len(argv)
        argv = myargv(*tuple(argv))
        res = libredrec.do_main(len(argv), argv, get_hmac_key_func, get_trace_key_func)
        sys.exit(res)
    except Exception, e:
        import traceback
        print "Failed to load redver library\n", traceback.format_exc(e)
        sys.exit(1)
