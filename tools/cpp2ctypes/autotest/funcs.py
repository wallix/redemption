from ctypes import CFUNCTYPE, POINTER, c_char_p, c_int, c_long, c_uint, c_ulonglong, c_void_p

# char const* version();
version.argtypes = []
version.restype = c_char_p

# int f1(int a, unsigned b, unsigned long long c, long d);
f1.argtypes = [c_int, c_uint, c_ulonglong, c_long]
f1.restype = c_int

# long* f2(MyType* my, char* d);
f2.argtypes = [c_void_p, c_char_p]
f2.restype = POINTER(c_long)

class CType_MyData(ctypes.Structure):
    _fields_ = [
        ("a", c_int),
        ("b", c_char_p),
    ]

# void f3(MyData const * my);
f3.argtypes = [POINTER(CType_MyData)]
f3.restype = None

Func1 = CFUNCTYPE(c_char_p, c_void_p, POINTER(c_size_t))

Func2 = CFUNCTYPE(c_char_p, c_void_p, c_size_t, c_char_p)

# int f4(Func1* f1, Func2* f2);
f4.argtypes = [Func1, Func2]
f4.restype = c_int

# int f4(Func1* f1, int n);
f4.argtypes = [Func1, c_int]
f4.restype = c_int

