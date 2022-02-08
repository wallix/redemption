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
    REDEMPTION_LIB_EXPORT int f4(Func1* f1, int n);
}
