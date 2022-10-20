from ctypes import CDLL, c_int, c_ubyte, c_uint
from enum import IntEnum

lib = CDLL("enums.so")

# C enum
# enum T1: int
#    {
#        // bla bla
#        A, B,
#    }
class T1(IntEnum):
    A = 0
    B = 1

    def from_param(self) -> int:
        return int(self)

lib.T1 = T1


# enum T2 : int {A,B,}
class T2(IntEnum):
    A = 0
    B = 1

    def from_param(self) -> int:
        return int(self)

lib.T2 = T2


# enum T3 : uint8_t

# enum [[nodiscard]] T4: unsigned{A,B,}
class T4(IntEnum):
    A = 0
    B = 1

    def from_param(self) -> int:
        return int(self)

lib.T4 = T4


# enum class
# enum class T5: unsigned {A,B,}
class T5(IntEnum):
    A = 0
    B = 1

    def from_param(self) -> int:
        return int(self)

lib.T5 = T5


# enum class T6 : unsigned{A,B,}
class T6(IntEnum):
    A = 0
    B = 1

    def from_param(self) -> int:
        return int(self)

lib.T6 = T6


# enum class [[nodiscard]] T7 : unsigned {A,B,}
class T7(IntEnum):
    A = 0
    B = 1

    def from_param(self) -> int:
        return int(self)

lib.T7 = T7


# void f1(T1 x);
f1 = lib.f1
f1.argtypes = [c_int]
f1.restype = None

# void f2(T2 x);
f2 = lib.f2
f2.argtypes = [c_int]
f2.restype = None

# void f3(T3 x);
f3 = lib.f3
f3.argtypes = [c_ubyte]
f3.restype = None

# void f4(T4 x);
f4 = lib.f4
f4.argtypes = [c_uint]
f4.restype = None

# void f5(T5 x);
f5 = lib.f5
f5.argtypes = [c_uint]
f5.restype = None

# void f6(T6 x);
f6 = lib.f6
f6.argtypes = [c_uint]
f6.restype = None

# void f7(T7 x);
f7 = lib.f7
f7.argtypes = [c_uint]
f7.restype = None

