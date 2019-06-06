"""
Here are defined callback functions called from C API
used to get hmac_key and trace_key.

The flag parameter is here to force use of old encryption scheme
where the master encoding key is kept inside wabengine
instead of being derivated. The key format also slightly changed.

"""

import ctypes

proxy = None

# def hexdumpkey(key):
#     return "".join(["\\x%s%s" % (hex((ord(x) >> 4)%16)[-1:], hex(ord(x)%16)[-1:]) for x in key])

def get_hmac_key(resbuf):
    try:
        if not proxy:
            return -1
        sign_key = proxy.get_trace_sign_key()
        # print "sign_key"+str(hexdumpkey(sign_key))
        ctypes.memmove(resbuf, sign_key, 32)
        return 0
    except:
        return -1

def get_trace_key(base, lg, resbuf, flag):
    try:
        if not proxy:
            return -1
        name = base[:lg]
        trace_key = proxy.get_trace_encryption_key(name, flag == 1)
        # print "trace_key"+str(hexdumpkey(trace_key))
        ctypes.memmove(resbuf, trace_key, 32)
        return 0
    except:
        return -1
