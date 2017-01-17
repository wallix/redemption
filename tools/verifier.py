#!/usr/bin/python -O
# -*- coding: utf-8 -*-
#

import ctypes
import ctypes.util
import sys
import os
import pwd

# If this code is not called from __main__
# You have to inject a proxy object in the module namespace

def get_proxy():
    configs = wabconfig.Config('wabengine')
    client = wabengine.client.sync_clientSynClient('localhost', configs['port'])
    try:
        proxy = client.authenticate_as_operator()
    except wabengine.common.AuthenticationFailed, exp:
        Logger().error("%s" % exp)
    return proxy

proxy = None
if __name__ == '__main__':
    try:
        from wabha.utils import *
        original_gid = os.getegid()
        original_uid = os.geteuid()

        os.setegid(pwd.getpwnam("wabuser").pw_gid)
        os.seteuid(pwd.getpwnam("wabuser").pw_uid)
        if ha_status() > HA_MASTER:
            # If HA is configured we run this script only on the MASTER node
            sys.exit(-1)

        proxy = get_proxy()

        os.seteuid(original_uid)
        os.setegid(original_gid)
    except:
        pass


GETHMACKEY = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_void_p)
GETTRACEKEY = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_char_p, ctypes.c_int, ctypes.c_void_p, ctypes.c_uint)

def get_hmac_key(resbuf):
    if proxy:
        sign_key = proxy.get_trace_sign_key()
        libredrec.recmemcpy(resbuf, sign_key, 32)
    return 0

def get_trace_key(base, lg, resbuf, flag):
    if proxy:
        name = base[:lg]
        trace_key = proxy.get_trace_encryption_key(name, flag==1)
        libredrec.recmemcpy(resbuf, trace_key, 32)
    return 0


get_hmac_key_func = GETHMACKEY(get_hmac_key)
get_trace_key_func = GETTRACEKEY(get_trace_key)
libredrec = ctypes.CDLL('libredrec.so')


def verify(cryptoapi, argv):
    from wabha.utils import cryptoapi
    args = ["redver"].extend(argv)
    global proxy
    proxy = cryptoapi
    tmpout = os.tmpfile()
    tmperr = os.tmpfile()
    stdoutfd = os.dup(sys.__stdout__.fileno())
    stderrfd = os.dup(sys.__stderr__.fileno())
    sys.stdout.flush()
    os.dup2(tmpout.fileno(), 1)
    sys.stderr.flush()
    os.dup2(tmperr.fileno(), 2)
    rcode = _verify(args)
    os.fsync(1)
    os.fsync(2)
    os.lseek(1, 0, os.SEEK_SET)
    os.lseek(2, 0, os.SEEK_SET)
    os.dup2(stdoutfd, 1)
    os.dup2(stderrfd, 2)
    sout = tmpout.readlines()
    serr = tmperr.readlines()
    return rcode, sout, serr

def _verify(sys_argv):
    libredrec.do_main.argtypes = [ctypes.c_uint, ctypes.POINTER(ctypes.c_char_p), GETHMACKEY, GETTRACEKEY]
    libredrec.do_main.restype = ctypes.c_int

    libredrec.recmemcpy.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
    libredrec.recmemcpy.restype = ctypes.c_int

    myargv = ctypes.c_char_p * len(sys_argv)
    argv = myargv(*tuple(sys_argv))
    res = libredrec.do_main(len(sys_argv), argv, get_hmac_key_func, get_trace_key_func)
    return res

if __name__ == '__main__':
    try:
        argv = sys.argv
        res = _verify(argv);
        sys.exit(res)
    except Exception, e:
        import traceback
        print "Failed to load redver library\n", traceback.format_exc(e)
        sys.exit(-1)
