Based on [Boost.Test](https://www.boost.org/doc/libs/1_70_0/libs/test/doc/html/index.html) with a `RED_` prefix rather than `BOOST_`.

(`docs/TU/unit_test.pdf`)

# test_framework/redemption_unit_tests.hpp

```cpp
RED_TEST_DONT_PRINT_LOG_VALUE(my_not_printable_type);
RED_TEST_DELEGATE_PRINT_ENUM(my_enum_type);

struct MyType { int x, y; };
RED_TEST_DELEGATE_PRINT(MyType, "MyType{" << x.x << ", " << x.y << "}");


RED_AUTO_TEST_CASE(test_name)
{
    RED_TEST(a == b);
    RED_TEST(a != b);
    RED_TEST(a < b);
    // etc

    RED_TEST(number == x +- 2_v); // number in [x-2, x+2]
    RED_TEST(number == x +- 2_percent); // number in [x-2%, x+2%]

    // "smart" print if error (ascii and/or hexadecimal)
    RED_TEST(view1 == view2);

    // output in ascii format
    RED_TEST(view1 == ut::ascii(view2, minimal_printable_ascii_char));
    RED_TEST(view1 == ut::ascii(view2/*, 0*/));
    RED_TEST(view1 == "abc"_av_ascii);

    // output in utf8 format
    RED_TEST(view1 == ut::utf8(view2));
    RED_TEST(view1 == "abc"_av_utf8);

    // output in hex format
    RED_TEST(view1 == ut::hex(view2));
    RED_TEST(view1 == "abc"_av_hex);

    // output in dump format
    RED_TEST(view1 == ut::dump(view2));
    RED_TEST(view1 == "abc"_av_dump);

    RED_TEST_MESSAGE(cond, "x = " << x);


    RED_CHECK_EQUAL_COLLECTIONS(cont1.begin(), cont1.end(), cont2.begin(), cont2.end());
    RED_CHECK_EQUAL_COLLECTIONS(cont1, cont2);


    RED_CHECK_PREDICATE(predicate_func, (param1)(param2)/*...*/);
    // equivalent to
    RED_CHECK_MESSAGE(predicate_func(param1, param2), "predicate_func(" << param1 << ", " << param2 ")");

    RED_TEST(RED_TEST_INVOKER(func)(x, y, /*...*/) == n);
    // or
    auto f = RED_TEST_INVOKER(func);
    RED_TEST(f(x, y, /*...*/) == n);
    // or
    auto f = RED_TEST_FUNC_CTX(func); /* RED_TEST_INVOKER without global capture */
    RED_TEST(f(x, y, /*...*/) == n);


    // check if throw with Error(ERR_TRANSPORT_DIFFERS)
    RED_CHECK_EXCEPTION_ERROR_ID(foo(), ERR_TRANSPORT_DIFFERS);

    RED_CHECK_THROW(foo(), my_exception);
    RED_CHECK_EXCEPTION(foo(), my_exception,
        ([](my_exception const& e) { return e.code() == 1; }]));
    RED_CHECK_NO_THROW(foo());


    RED_TEST_CONTEXT("with x " << x)
    {
        // ...
    }

    struct Data
    {
        int x;
        int result;
    };
    RED_TEST_CONTEXT_DATA(Data const& data, "x = " << data.x /* ostream context expression*/, {
        Data{1, 3},
        Data{2, 2},
        // ...
    })
    {
        RED_CHECK(foo(data.x), data.result);
    }

    RED_TEST_DATAS(
        (param1, param2) // first context
        (param1, param2) // second context
        // etc
    ) >>= [&](auto p1, auto p2){
        RED_CHECK(foo(p1), p2);
    };

    RED_FAIL("because x = " << x);
    RED_ERROR("because x = " << x);
    RED_TEST_CHECKPOINT("because x = " << x);
    RED_TEST_PASSPOINT();

    while (!RED_ERROR_COUNT()) {
        // ...
    }
}
```


# test_framework/file.hpp

```cpp
RED_AUTO_TEST_CASE(test_name)
{
    RED_TEST_FILE_SIZE("path", len);
    RED_TEST_FILE_SIZE("path", len +- 100_v);
    RED_TEST_FILE_SIZE("path", len +- 2_percent);

    auto s = RED_CHECK_GET_FILE_CONTENTS("filename");
    RED_TEST(s == "bla bla");

    // or

    RED_TEST_FILE_CONTENTS("filename", "bla bla");
}
```

# test_framework/working_directory.hpp

```cpp
RED_AUTO_TEST_CASE(test_name)
{
    WorkingFile wf("test_name");
    // ...
    // ~WorkingFile -> check if "filename" exists or not with wf.set_removed()
}

// shortcut
RED_AUTO_TEST_CASE_WF(test_name, wf)
{
    // ...
}

RED_AUTO_TEST_CASE(test_name)
{
    WorkingDirectory wd("test_name");

    auto path1 = wd.add_file("file1");
    auto path2 = wd.add_file("file2");
    wd.add_files({"file3", "file4"/*, ...*/});
    auto subdir = wd.create_subdirectory("sub1");
    auto path3 = subdir.add_file("file5");

    test_create_files(wd.dirname());

    RED_CHECK_WORKSPACE(wd);

    wd.remove_files({"file1", "file3"});
    wd.remove_file("file2");

    // ~WorkingDirectory -> RED_CHECK_WORKSPACE(wd)
}

// shortcut
RED_AUTO_TEST_CASE_WD(test_name, wd)
{
    // ...
}
```

# test_framework/hex.hpp

```cpp
RED_AUTO_TEST_CASE(test_name)
{
    RED_TEST(ut:hex16{4} == 3); // 0x0004 != 0x0003
    RED_TEST(ut:hex_minimal{4} == 3); // 0x04 != 0x03
}
```
