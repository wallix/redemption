from ctypes import CDLL, c_int, c_ubyte, c_uint

lib = CDLL("enums.so")


# C enum
# enum T1: int
#    {
#        // bla bla
#        A, B,
#    }
# enum T2 : int {A,B,}
# enum T3 : uint8_t
# enum [[nodiscard]] T4: unsigned{A,B,}
# enum class
# enum class T5: unsigned {A,B,}
# enum class T6 : unsigned{A,B,}
# enum class [[nodiscard]] T7 : unsigned {A,B,}
# void f1(T1 x);
lib.f1.argtypes = [c_int]
lib.f1.restype = None

# void f2(T2 x);
lib.f2.argtypes = [c_int]
lib.f2.restype = None

# void f3(T3 x);
lib.f3.argtypes = [c_ubyte]
lib.f3.restype = None

# void f4(T4 x);
lib.f4.argtypes = [c_uint]
lib.f4.restype = None

# void f5(T5 x);
lib.f5.argtypes = [c_uint]
lib.f5.restype = None

# void f6(T6 x);
lib.f6.argtypes = [c_uint]
lib.f6.restype = None

# void f7(T7 x);
lib.f7.argtypes = [c_uint]
lib.f7.restype = None

