# A.v1 = 3
# A.xxx = 4
# A.yy = 3
# A.Z = 4
# A.Y = 5
A = lib.A

class MyType:
    __slot__ = ('_ctx')

    def __init__(self, x:int) -> None:
        self._ctx = lib.my_type_new(x)
        if not self._ctx:
            raise Exception("malloc error")

    def __del__(self) -> None:
        lib.my_type_delete(self._ctx)

    def get(self, default_value:int) -> int:
        return lib.my_type_get(self._ctx, default_value)

    def foo(self, a:A) -> None:
        lib.my_type_foo(self._ctx, int(a))


