from ctypes import CDLL, CFUNCTYPE, POINTER, c_char, c_char_p, c_int, c_int64, c_long, c_longlong, c_size_t, c_uint, c_ulong, c_ulonglong, c_void_p

lib = CDLL("funcs.so")

# char const* version();
version = lib.version
version.argtypes = []
version.restype = c_char_p

# int f1(int a, unsigned b, unsigned long long c, long d);
f1 = lib.f1
f1.argtypes = [c_int, c_uint, c_ulonglong, c_long]
f1.restype = c_int

# long* f2(MyType* my, char* d);
f2 = lib.f2
f2.argtypes = [c_void_p, c_char_p]
f2.restype = POINTER(c_long)

class CType_MyData(ctypes.Structure):
    _fields_ = [
        ("a", c_int),
        ("b", c_char_p),
    ]

# void f3(MyData const * my);
f3 = lib.f3
f3.argtypes = [POINTER(CType_MyData)]
f3.restype = None

Func1 = CFUNCTYPE(c_char_p, c_void_p, POINTER(c_size_t))

Func2 = CFUNCTYPE(c_char_p, c_void_p, c_size_t, c_char_p)

# int f4(Func1* f1, Func2* f2);
f4 = lib.f4
f4.argtypes = [c_void_p, c_void_p]
f4.restype = c_int

# int f5(Func1* f1, int n);
f5 = lib.f5
f5.argtypes = [c_void_p, c_int]
f5.restype = c_int

# string
# int f6(char const* str);
f6 = lib.f6
f6.argtypes = [c_char_p]
f6.restype = c_int

# buffer
# int f7(uint8_t const* buffer);
f7 = lib.f7
f7.argtypes = [POINTER(c_char)]
f7.restype = c_int

# long long f8(unsigned long len);
f8 = lib.f8
f8.argtypes = [c_ulong]
f8.restype = c_longlong

# unsigned long f9(unsigned long len);
f9 = lib.f9
f9.argtypes = [c_ulong]
f9.restype = c_ulong

# long long f10(int64_t len);
f10 = lib.f10
f10.argtypes = [c_int64]
f10.restype = c_longlong

# long long f11(unsigned long long len);
f11 = lib.f11
f11.argtypes = [c_ulonglong]
f11.restype = c_longlong

