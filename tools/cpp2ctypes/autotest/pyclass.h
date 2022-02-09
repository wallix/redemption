extern "C" {
    REDEMPTION_LIB_EXPORT char const* version();
    REDEMPTION_LIB_EXPORT int f1(int a, unsigned b, unsigned long long c, long d);

    struct MyType;

    REDEMPTION_LIB_EXPORT MyType* my_type_new(int x);
    REDEMPTION_LIB_EXPORT void my_type_delete(MyType* my);
    REDEMPTION_LIB_EXPORT int my_type_get(MyType* my, int default_value);

    enum class A : int
    {
        v1 = 3,
        xxx = 4,
        yy = v1,
        Z,
        Y,
    };

    REDEMPTION_LIB_EXPORT void my_type_foo(MyType* my, A a);
}
