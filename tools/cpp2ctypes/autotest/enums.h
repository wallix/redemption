extern "C"
{
    // C enum
    enum T1:int
    {
        // bla bla
        A, B,
    };
    enum T2 : int {A,B,};
    enum T3 : uint8_t;
    enum [[nodiscard]] T4:unsigned{A,B,};
    // enum class
    enum class T5:unsigned {A,B,};
    enum class T6 :unsigned{A,B,};
    enum class [[nodiscard]] T7 : unsigned {A,B,};

    REDEMPTION_LIB_EXPORT void f1(T1 x);
    REDEMPTION_LIB_EXPORT void f2(T2 x);
    REDEMPTION_LIB_EXPORT void f3(T3 x);
    REDEMPTION_LIB_EXPORT void f4(T4 x);
    REDEMPTION_LIB_EXPORT void f5(T5 x);
    REDEMPTION_LIB_EXPORT void f6(T6 x);
    REDEMPTION_LIB_EXPORT void f7(T7 x);
}
