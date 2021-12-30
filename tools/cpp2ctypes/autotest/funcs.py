from ctypes import c_char_p, c_int, c_long, c_uint, c_ulonglong, c_void_p

# char const* version();
version.argtypes = []
version.restype = c_char_p

# int f1(int a, unsigned b, unsigned long long c, long d);
f1.argtypes = [c_int, c_uint, c_ulonglong, c_long]
f1.restype = c_int

# long* f2(MyType* my, char* d);
f2.argtypes = [c_void_p, c_char_p]
f2.restype = POINTER(c_long)

