extern "C" {
    REDEMPTION_LIB_EXPORT char const* version();
    REDEMPTION_LIB_EXPORT int f1(int a, unsigned b, unsigned long long c, long d);

    struct MyType;
    REDEMPTION_LIB_EXPORT long* f2(MyType* my, char* d);

    struct MyData
    {
        int a;
        char* b;
    };

    REDEMPTION_LIB_EXPORT void f3(MyData const * my);

    using Func1 = char*(void* ctx, std::size_t * output_len) noexcept;
    using Func2 = char*(void* ctx, std::size_t n, char*);

    REDEMPTION_LIB_EXPORT int f4(Func1* f1, Func2* f2);
    REDEMPTION_LIB_EXPORT int f5(Func1* f1, int n);

    // string
    REDEMPTION_LIB_EXPORT int f6(char const* str);
    // buffer
    REDEMPTION_LIB_EXPORT int f7(uint8_t const* buffer);

    REDEMPTION_LIB_EXPORT long long f8(unsigned long len);
    REDEMPTION_LIB_EXPORT unsigned long f9(unsigned long len);

    REDEMPTION_LIB_EXPORT long long f10(int64_t len);
    REDEMPTION_LIB_EXPORT long long f11(unsigned long long len);
}
