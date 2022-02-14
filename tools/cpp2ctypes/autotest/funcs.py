from ctypes import CDLL, CFUNCTYPE, POINTER, c_char, c_char_p, c_int, c_long, c_size_t, c_uint, c_ulonglong, c_void_p

lib = CDLL("funcs.so")

# char const* version();
lib.version.argtypes = []
lib.version.restype = c_char_p

# int f1(int a, unsigned b, unsigned long long c, long d);
lib.f1.argtypes = [c_int, c_uint, c_ulonglong, c_long]
lib.f1.restype = c_int

# long* f2(MyType* my, char* d);
lib.f2.argtypes = [c_void_p, c_char_p]
lib.f2.restype = POINTER(c_long)

class CType_MyData(ctypes.Structure):
    _fields_ = [
        ("a", c_int),
        ("b", c_char_p),
    ]

# void f3(MyData const * my);
lib.f3.argtypes = [POINTER(CType_MyData)]
lib.f3.restype = None

Func1 = CFUNCTYPE(c_char_p, c_void_p, POINTER(c_size_t))
lib.Func1 = Func1

Func2 = CFUNCTYPE(c_char_p, c_void_p, c_size_t, c_char_p)
lib.Func2 = Func2

# int f4(Func1* f1, Func2* f2);
lib.f4.argtypes = [Func1, Func2]
lib.f4.restype = c_int

# int f5(Func1* f1, int n);
lib.f5.argtypes = [Func1, c_int]
lib.f5.restype = c_int

# string
# int f6(char const* str);
lib.f6.argtypes = [c_char_p]
lib.f6.restype = c_int

# buffer
# int f7(uint8_t const* buffer);
lib.f7.argtypes = [POINTER(c_char)]
lib.f7.restype = c_int

