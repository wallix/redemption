from ctypes import c_int, c_uint, c_uint8

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
f1.argtypes = [c_int]
f1.restype = None

# void f2(T2 x);
f2.argtypes = [c_int]
f2.restype = None

# void f3(T3 x);
f3.argtypes = [c_uint8]
f3.restype = None

# void f4(T4 x);
f4.argtypes = [c_uint]
f4.restype = None

# void f5(T5 x);
f5.argtypes = [c_uint]
f5.restype = None

# void f6(T6 x);
f6.argtypes = [c_uint]
f6.restype = None

# void f7(T7 x);
f7.argtypes = [c_uint]
f7.restype = None

