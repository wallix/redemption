/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Meng Tan
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to rect object

*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/compare_collection.hpp"


#include "utils/cli.hpp"
#include "utils/cli_chrono.hpp"


namespace cli
{
    static bool operator == (ParseResult const& r1, ParseResult const& r2) {
        return r1.res == r2.res
            && r1.argc == r2.argc
            && r1.argv == r2.argv
            && r1.opti == r2.opti
            // && r1.str == r2.str
            ;
    }
}

#if !REDEMPTION_UNIT_TEST_FAST_CHECK
static ut::assertion_result parse_result_eq(
    ::cli::ParseResult const& r1, ::cli::ParseResult const& r2)
{
    auto put = [&](std::ostream& out, ::cli::ParseResult const& r){
        redemption_unit_test__::Enum e(r.res);
        std::string_view value_name = {e.value_name.data(), e.value_name.size()};
        out << "{ .opti=" << r.opti
            << ", .argc=" << r.argc
            << ", .argv=" << r.argv
            // << ", .str=" << (r.str ? r.str : "nullptr")
            << ", .res=" << value_name
            << " }";
    };

    return ut::create_assertion_result(r1 == r2, r1, " != ", r2, put);
}

RED_TEST_DISPATCH_COMPARISON_EQ((), (::cli::ParseResult), (::cli::ParseResult), ::parse_result_eq)
#endif

RED_AUTO_TEST_CASE(TestCLI_fn_arg)
{
    int i = 0;
    auto p1 = cli::arg([&]{ i = 1; });
    auto p2 = cli::arg([&](int x){ i = x; });

    cli::ParseResult pr{};

    pr.str = nullptr;
    RED_CHECK(cli::Res::Ok == p1(pr));
    RED_CHECK(i == 1);

    pr.str = "3";
    RED_CHECK(cli::Res::Ok == p2(pr));
    RED_CHECK(i == 3);
}

RED_AUTO_TEST_CASE(TestCLI_parse_short_required_option)
{
    char const* argv[] {"progname", "-x1", "-x=2", "-x", "3", "-xx", ""};
    const int argc = int(std::size(argv))-1;

    auto mk_pr = [&](int opti){
        cli::ParseResult pr;
        pr.argc = argc;
        pr.argv = argv;
        pr.opti = opti;
        pr.str = nullptr;
        return pr;
    };

    int n1 = 0;

    const auto opt = cli::option('x').parser(cli::arg_location(n1));

    {
        cli::ParseResult pr = mk_pr(1);
        auto res = cli::Res::Ok;
        RED_CHECK(!cli::detail::parse_short_option(argv[1]+1, pr, opt, res));
        RED_CHECK(res == cli::Res::Ok);
        RED_CHECK(pr.opti == 2);
        RED_CHECK(n1 == 1);
    }

    {
        cli::ParseResult pr = mk_pr(2);
        auto res = cli::Res::Ok;
        RED_CHECK(!cli::detail::parse_short_option(argv[2]+1, pr, opt, res));
        RED_CHECK(res == cli::Res::Ok);
        RED_CHECK(pr.opti == 3);
        RED_CHECK(n1 == 2);
    }

    {
        cli::ParseResult pr = mk_pr(3);
        auto res = cli::Res::Ok;
        RED_CHECK(!cli::detail::parse_short_option(argv[3]+1, pr, opt, res));
        RED_CHECK(res == cli::Res::Ok);
        RED_CHECK(pr.opti == 5);
        RED_CHECK(n1 == 3);
    }

    {
        cli::ParseResult pr = mk_pr(5);
        auto res = cli::Res::Ok;
        RED_CHECK(!cli::detail::parse_short_option(argv[5]+1, pr, opt, res));
        RED_CHECK(res == cli::Res::BadFormat);
        RED_CHECK(pr.opti == 5);
    }
}

RED_AUTO_TEST_CASE(TestCLI_parse_short_optional_option)
{
    char const* argv[] {"progname", "-x", "-x=off", "-xon", "-xx", ""};
    const int argc = int(std::size(argv))-1;

    auto mk_pr = [&](int opti){
        cli::ParseResult pr;
        pr.argc = argc;
        pr.argv = argv;
        pr.opti = opti;
        pr.str = nullptr;
        return pr;
    };

    int n1 = 2;

    const auto opt = cli::option('x').parser(cli::on_off_location(n1));

    {
        cli::ParseResult pr = mk_pr(1);
        auto res = cli::Res::Ok;
        RED_CHECK(!cli::detail::parse_short_option(argv[1]+1, pr, opt, res));
        RED_CHECK(res == cli::Res::Ok);
        RED_CHECK(pr.opti == 2);
        RED_CHECK(n1 == 1);
    }

    {
        cli::ParseResult pr = mk_pr(2);
        auto res = cli::Res::Ok;
        RED_CHECK(!cli::detail::parse_short_option(argv[2]+1, pr, opt, res));
        RED_CHECK(res == cli::Res::Ok);
        RED_CHECK(pr.opti == 3);
        RED_CHECK(n1 == 0);
    }

    {
        cli::ParseResult pr = mk_pr(3);
        auto res = cli::Res::Ok;
        RED_CHECK(!cli::detail::parse_short_option(argv[3]+1, pr, opt, res));
        RED_CHECK(res == cli::Res::Ok);
        RED_CHECK(pr.opti == 4);
        RED_CHECK(n1 == 1);
    }

    {
        cli::ParseResult pr = mk_pr(4);
        auto res = cli::Res::Ok;
        RED_CHECK(!cli::detail::parse_short_option(argv[4]+1, pr, opt, res));
        RED_CHECK(res == cli::Res::BadFormat);
        RED_CHECK(pr.opti == 4);
    }
}

RED_AUTO_TEST_CASE(TestCLI_parse_short_novalue_option)
{
    char const* argv[] {"progname", "-x", "-xx", "-x=1", ""};
    const int argc = int(std::size(argv))-1;

    auto mk_pr = [&](int opti){
        cli::ParseResult pr;
        pr.argc = argc;
        pr.argv = argv;
        pr.opti = opti;
        pr.str = nullptr;
        return pr;
    };

    int n1 = 0;

    const auto opt = cli::option('x').parser(cli::arg_increment(n1));

    {
        cli::ParseResult pr = mk_pr(1);
        auto res = cli::Res::Ok;
        RED_CHECK(!cli::detail::parse_short_option(argv[1]+1, pr, opt, res));
        RED_CHECK(res == cli::Res::Ok);
        RED_CHECK(pr.opti == 2);
        RED_CHECK(n1 == 1);
    }

    {
        cli::ParseResult pr = mk_pr(2);
        auto res = cli::Res::Ok;
        RED_CHECK(!cli::detail::parse_short_option(argv[2]+1, pr, opt, res));
        RED_CHECK(res == cli::Res::Ok);
        RED_CHECK(pr.opti == 2);
        RED_CHECK(n1 == 2);
    }

    {
        cli::ParseResult pr = mk_pr(3);
        auto res = cli::Res::Ok;
        RED_CHECK(!cli::detail::parse_short_option(argv[3]+1, pr, opt, res));
        RED_CHECK(res == cli::Res::BadFormat);
    }
}

RED_AUTO_TEST_CASE(TestCLI_parse_long_required_option)
{
    char const* argv[] {"progname", "--xxx=1", "--xxx", "2", "--xxx3", ""};
    const int argc = int(std::size(argv))-1;

    auto mk_pr = [&](int opti){
        cli::ParseResult pr;
        pr.argc = argc;
        pr.argv = argv;
        pr.opti = opti;
        pr.str = nullptr;
        return pr;
    };

    int n1 = 0;

    const auto opt = cli::option("xxx").parser(cli::arg_location(n1));

    {
        cli::ParseResult pr = mk_pr(1);
        auto res = cli::Res::Ok;
        RED_CHECK(!cli::detail::parse_long_option({argv[1]+2, 3}, pr, opt, res));
        RED_CHECK(res == cli::Res::Ok);
        RED_CHECK(pr.opti == 2);
        RED_CHECK(n1 == 1);
    }

    {
        cli::ParseResult pr = mk_pr(2);
        auto res = cli::Res::Ok;
        RED_CHECK(!cli::detail::parse_long_option(argv[2]+2, pr, opt, res));
        RED_CHECK(res == cli::Res::Ok);
        RED_CHECK(pr.opti == 4);
        RED_CHECK(n1 == 2);
    }

    {
        cli::ParseResult pr = mk_pr(4);
        auto res = cli::Res::Ok;
        RED_CHECK(cli::detail::parse_long_option(argv[4]+2, pr, opt, res));
        RED_CHECK(res == cli::Res::Ok);
        RED_CHECK(pr.opti == 4);
    }
}

RED_AUTO_TEST_CASE(TestCLI_parse_long_optional_option)
{
    char const* argv[] {"progname", "--xxx", "--xxx=off", "--xxxon", ""};
    const int argc = int(std::size(argv))-1;

    auto mk_pr = [&](int opti){
        cli::ParseResult pr;
        pr.argc = argc;
        pr.argv = argv;
        pr.opti = opti;
        pr.str = nullptr;
        return pr;
    };

    int n1 = 2;

    const auto opt = cli::option("xxx").parser(cli::on_off_location(n1));

    {
        cli::ParseResult pr = mk_pr(1);
        auto res = cli::Res::Ok;
        RED_CHECK(!cli::detail::parse_long_option(argv[1]+2, pr, opt, res));
        RED_CHECK(res == cli::Res::Ok);
        RED_CHECK(pr.opti == 2);
        RED_CHECK(n1 == 1);
    }

    {
        cli::ParseResult pr = mk_pr(2);
        auto res = cli::Res::Ok;
        RED_CHECK(!cli::detail::parse_long_option({argv[2]+2, 3}, pr, opt, res));
        RED_CHECK(res == cli::Res::Ok);
        RED_CHECK(pr.opti == 3);
        RED_CHECK(n1 == 0);
    }

    {
        cli::ParseResult pr = mk_pr(3);
        auto res = cli::Res::Ok;
        RED_CHECK(cli::detail::parse_long_option(argv[3]+2, pr, opt, res));
        RED_CHECK(res == cli::Res::Ok);
        RED_CHECK(pr.opti == 3);
    }
}

RED_AUTO_TEST_CASE(TestCLI_parse_long_novalue_option)
{
    char const* argv[] {"progname", "--xxx", "--xxxx", "--xxx=1", ""};
    const int argc = int(std::size(argv))-1;

    auto mk_pr = [&](int opti){
        cli::ParseResult pr;
        pr.argc = argc;
        pr.argv = argv;
        pr.opti = opti;
        pr.str = nullptr;
        return pr;
    };

    int n1 = 0;

    const auto opt = cli::option("xxx").parser(cli::arg_increment(n1));

    {
        cli::ParseResult pr = mk_pr(1);
        auto res = cli::Res::Ok;
        RED_CHECK(!cli::detail::parse_long_option(argv[1]+2, pr, opt, res));
        RED_CHECK(res == cli::Res::Ok);
        RED_CHECK(pr.opti == 2);
        RED_CHECK(n1 == 1);
    }

    {
        cli::ParseResult pr = mk_pr(2);
        auto res = cli::Res::Ok;
        RED_CHECK(cli::detail::parse_long_option(argv[2]+2, pr, opt, res));
        RED_CHECK(res == cli::Res::Ok);
    }

    {
        cli::ParseResult pr = mk_pr(3);
        auto res = cli::Res::Ok;
        RED_CHECK(!cli::detail::parse_long_option({argv[3]+2, 3}, pr, opt, res));
        RED_CHECK(res == cli::Res::BadOption);
    }
}

RED_AUTO_TEST_CASE(TestCLI_parse_unknown_option)
{
    char const* argv[] {"progname", "--yyy", "-y", ""};
    const int argc = int(std::size(argv))-1;

    auto mk_pr = [&](int opti){
        cli::ParseResult pr;
        pr.argc = argc;
        pr.argv = argv;
        pr.opti = opti;
        pr.str = nullptr;
        return pr;
    };

    int n1 = 0;

    const auto opt = cli::option('x',"xxx").parser(cli::arg_increment(n1));

    {
        cli::ParseResult pr = mk_pr(1);
        auto res = cli::Res::Ok;
        RED_CHECK(cli::detail::parse_long_option(argv[1]+2, pr, opt, res));
    }

    {
        cli::ParseResult pr = mk_pr(2);
        auto res = cli::Res::Ok;
        RED_CHECK(cli::detail::parse_long_option(argv[1]+2, pr, opt, res));
    }

}

RED_AUTO_TEST_CASE(TestCLI_arg_location)
{
    int n1 = 0;
    int n2 = 0;
    int n3 = 0;
    auto options = cli::options(
        cli::option('x').parser(cli::arg_location(n1)).help("bla bla bla"),
        cli::option('y', "yy").parser(cli::arg_location(n2)).argname("<b>").help("bla bla bla"),
        cli::option("z").argname("<c>").parser(cli::arg_location(n3)).help("bla bla bla")
    );

    {
        char const* argv[] {"progname", "-x1", "--z=3", ""};
        const int argc = int(std::size(argv))-1;
        RED_CHECK(cli::parse(options, argc, argv) == (cli::ParseResult{argc, argc, argv, nullptr, cli::Res::Ok}));
        RED_CHECK(n1 == 1);
        RED_CHECK(n2 == 0);
        RED_CHECK(n3 == 3);
    }
    {
        char const* argv[] {"progname", "-x=4", "--yy=5", ""};
        const int argc = int(std::size(argv))-1;
        RED_CHECK(cli::parse(options, argc, argv) == (cli::ParseResult{argc, argc, argv, nullptr, cli::Res::Ok}));
        RED_CHECK(n1 == 4);
        RED_CHECK(n2 == 5);
        RED_CHECK(n3 == 3);
    }

    std::ostringstream oss;
    cli::print_help(options, oss);
    RED_CHECK(oss.str() ==
        "  -x=<value>      bla bla bla\n"
        "  -y, --yy=<b>    bla bla bla\n"
        "  --z=<c>         bla bla bla\n");
}

RED_AUTO_TEST_CASE(TestCLI_on_off)
{
    int n1 = 2;
    int n2 = 2;
    int n3 = 2;
    auto options = cli::options(
        cli::option('x').parser(cli::on_off_location(n1)).help("bla bla bla"),
        cli::option('y', "yy").parser(cli::on_off_location(n2)).help("bla bla bla"),
        cli::option("z").parser(cli::on_off_location(n3)).help("bla bla bla")
    );

    {
        char const* argv[] {"progname", "-x", ""};
        const int argc = int(std::size(argv))-1;
        RED_CHECK(cli::parse(options, argc, argv) == (cli::ParseResult{argc, argc, argv, nullptr, cli::Res::Ok}));
        RED_CHECK(n1 == 1);
        RED_CHECK(n2 == 2);
        RED_CHECK(n3 == 2);
    }
    {
        char const* argv[] {"progname", "-xoff", "--z=on", ""};
        const int argc = int(std::size(argv))-1;
        RED_CHECK(cli::parse(options, argc, argv) == (cli::ParseResult{argc, argc, argv, nullptr, cli::Res::Ok}));
        RED_CHECK(n1 == 0);
        RED_CHECK(n2 == 2);
        RED_CHECK(n3 == 1);
    }

    std::ostringstream oss;
    cli::print_help(options, oss);
    RED_CHECK(oss.str() ==
        "  -x[={on|off}]          bla bla bla\n"
        "  -y, --yy[={on|off}]    bla bla bla\n"
        "  --z[={on|off}]         bla bla bla\n");
}

RED_AUTO_TEST_CASE(TestCLI_trigger)
{
    int n1 = 0;
    int n2 = 0;
    int n3 = 0;
    auto options = cli::options(
        cli::option('x').parser(cli::arg_increment(n1)).help("bla bla bla"),
        cli::option('y', "yy").parser(cli::arg_increment(n2)).help("bla bla bla"),
        cli::option("z").parser(cli::arg_increment(n3)).help("bla bla bla")
    );

    {
        char const* argv[] {"progname", "-xyx", ""};
        const int argc = int(std::size(argv))-1;
        RED_CHECK(cli::parse(options, argc, argv) == (cli::ParseResult{argc, argc, argv, nullptr, cli::Res::Ok}));
        RED_CHECK(n1 == 2);
        RED_CHECK(n2 == 1);
        RED_CHECK(n3 == 0);
    }
    {
        char const* argv[] {"progname", "-xxyy", "--z", ""};
        const int argc = int(std::size(argv))-1;
        RED_CHECK(cli::parse(options, argc, argv) == (cli::ParseResult{argc, argc, argv, nullptr, cli::Res::Ok}));
        RED_CHECK(n1 == 4);
        RED_CHECK(n2 == 3);
        RED_CHECK(n3 == 1);
    }

    std::ostringstream oss;
    cli::print_help(options, oss);
    RED_CHECK(oss.str() ==
        "  -x          bla bla bla\n"
        "  -y, --yy    bla bla bla\n"
        "  --z         bla bla bla\n");
}

RED_AUTO_TEST_CASE(TestCLI_password1)
{
    std::string password;

    char argv0[] = "progname";
    char argv1[] = "-p";
    char argv2[] = "pass";
    char argv3[] = "";
    char* argv[] {argv0, argv1, argv2, argv3, };
    const int argc = int(std::size(argv))-1;

    auto options = cli::options(
        cli::option('p').parser(cli::password_location(argv, password)).help("bla bla bla")
    );
    RED_CHECK(cli::parse(options, argc, argv) == (cli::ParseResult{argc, argc, argv, nullptr, cli::Res::Ok}));
    RED_CHECK(password == "pass");
    RED_CHECK(bytes_view(argv2, std::size(argv2)) == "*\0\0\0\0"_av);

    std::ostringstream oss;
    cli::print_help(options, oss);
    RED_CHECK(oss.str() ==
        "  -p=<password>    bla bla bla\n");
}

RED_AUTO_TEST_CASE(TestCLI_password2)
{
    std::string password;

    char argv0[] = "progname";
    char argv1[] = "-ppass";
    char argv2[] = "";
    char* argv[] {argv0, argv1, argv2, };
    const int argc = int(std::size(argv))-1;

    auto options = cli::options(
        cli::option('p').parser(cli::password_location(argv, password)).help("bla bla bla")
    );
    RED_CHECK(cli::parse(options, argc, argv) == (cli::ParseResult{argc, argc, argv, nullptr, cli::Res::Ok}));
    RED_CHECK(password == "pass");
    RED_CHECK(bytes_view(argv1, std::size(argv1)) == "-p*\0\0\0\0"_av);

    std::ostringstream oss;
    cli::print_help(options, oss);
    RED_CHECK(oss.str() ==
        "  -p=<password>    bla bla bla\n");
}

RED_AUTO_TEST_CASE(TestCLI_positional)
{
    bool a = false;
    auto options = cli::options(
        cli::option("a").parser(cli::arg_triggered(a))
    );

    {
        char const* argv[] {"progname", "--", ""};
        const int argc = int(std::size(argv))-1;
        auto pr = cli::parse(options, argc, argv);
        RED_CHECK(pr == (cli::ParseResult{1, argc, argv, nullptr, cli::Res::StopParsing}));
    }

    {
        char const* argv[] {"progname", "a", ""};
        const int argc = int(std::size(argv))-1;
        auto pr = cli::parse(options, argc, argv);
        RED_CHECK(pr == (cli::ParseResult{1, argc, argv, nullptr, cli::Res::NotOption}));
    }
}

RED_AUTO_TEST_CASE(TestCLI_conflict)
{
    bool a = false;
    bool b = false;
    auto options = cli::options(
        cli::option("abc").parser(cli::arg_triggered(a)),
        cli::option("abcd").parser(cli::arg_triggered(b))
    );

    char const* argv[] {"progname", "--abcd", ""};
    const int argc = int(std::size(argv))-1;
    RED_CHECK(cli::parse(options, argc, argv) == (cli::ParseResult{argc, argc, argv, nullptr, cli::Res::Ok}));
    RED_CHECK(b);
}

RED_AUTO_TEST_CASE(TestCLI_help)
{
    int x;
    auto options = cli::options(
        cli::option('a').parser(cli::arg_location(x)),
        cli::option('b').parser(cli::arg_location(x)).argname("<A>"),
        cli::helper("Password options:"),
        cli::option('c').parser(cli::password_location(nullptr, x)),
        cli::option('d').parser(cli::password_location(nullptr, x)).argname("<P>"),
        cli::option('e').parser(cli::arg([](std::chrono::milliseconds /*ms*/){})),
        cli::option('f').parser(cli::arg([](std::chrono::milliseconds /*ms*/){})).argname("<C>")
    );

    char const* argv[] {"progname", ""};
    const int argc = int(std::size(argv))-1;
    RED_CHECK(cli::parse(options, argc, argv) == (cli::ParseResult{argc, argc, argv, nullptr, cli::Res::Ok}));

    std::ostringstream oss;
    cli::print_help(options, oss);
    RED_CHECK(oss.str() ==
        "  -a=<value>\n"
        "  -b=<A>\n"
        "\nPassword options:\n\n"
        "  -c=<password>\n"
        "  -d=<P>\n"
        "  -e=<milliseconds>\n"
        "  -f=<C>\n"_av);
}
