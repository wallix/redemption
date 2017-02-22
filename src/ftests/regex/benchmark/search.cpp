/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#include "basic_benchmark.hpp"
#include "regex/regex.hpp"

#include <regex.h>
#include <pcre.h>

struct test_search
{
    re::Regex regex;

    test_search(const char * pattern, bool optimize_mem = false)
    : regex(pattern, optimize_mem)
    {}

    bool check_pre_condition(const char * s)
    {
        return this->regex.message_error() == 0 && this->regex.search(s);
    }

    void exec(const char * s)
    {
        this->regex.search(s);
    }
};

struct test_search_optimize_mem
: test_search
{
    test_search_optimize_mem(const char* pattern)
    : test_search(pattern, true)
    {}
};

struct test_search_capture : test_search
{
    test_search_capture(const char * pattern, bool optimize_mem = false)
    : test_search(pattern, optimize_mem)
    {}

    void exec(const char * s)
    {
        this->regex.search_with_matches(s);
    }
};

struct test_search_capture_optimize_mem
: test_search_capture
{
    test_search_capture_optimize_mem(const char* pattern)
    : test_search_capture(pattern, true)
    {}
};

struct test_pcre_search
{
    pcre * rgx;

    test_pcre_search(const char * pattern)
    {
        const char * err = 0;
        int iffset = 0;
        this->rgx = pcre_compile(
            pattern,              /* the pattern */
            0,                    /* default options */
            &err,               /* for error message */
            &iffset,           /* for error offset */
            NULL);                /* use default character tables */
    }

    ~test_pcre_search()
    {
        pcre_free(this->rgx);
    }

    bool check_pre_condition(const char * s)
    {
        int ovector[30];
        return this->rgx && pcre_exec(
            this->rgx,                   /* the compiled pattern */
            NULL,                 /* no extra data - we didn't study the pattern */
            s,              /* the subject string */
            strlen(s),       /* the length of the subject */
            0,                    /* start at offset 0 in the subject */
            0,                    /* default options */
            ovector,              /* output vector for substring information */
            30)>=0;
    }

    void exec(const char * s)
    {
        int ovector[30];
        pcre_exec(
            this->rgx,                   /* the compiled pattern */
            NULL,                 /* no extra data - we didn't study the pattern */
            s,              /* the subject string */
            strlen(s),       /* the length of the subject */
            0,                    /* start at offset 0 in the subject */
            0,                    /* default options */
            ovector,              /* output vector for substring information */
            30);           /* number of elements in the output vector */
    }
};

struct test_pcre_search_capture : test_pcre_search
{
    int regmatch[30]; /* should be a multiple of 3 */

    test_pcre_search_capture(const char * pattern)
    : test_pcre_search(pattern)
    {}

    bool check_pre_condition(const char * s)
    {
      return this->rgx && pcre_exec(
        this->rgx,                   /* the compiled pattern */
        NULL,                 /* no extra data - we didn't study the pattern */
        s,              /* the subject string */
        strlen(s),       /* the length of the subject */
        0,                    /* start at offset 0 in the subject */
        0,                    /* default options */
        this->regmatch,              /* output vector for substring information */
        sizeof(this->regmatch)/sizeof(this->regmatch[0]))>=0;
    }

    void exec(const char * s)
    {
        pcre_exec(
            this->rgx,                   /* the compiled pattern */
            NULL,                 /* no extra data - we didn't study the pattern */
            s,              /* the subject string */
            strlen(s),       /* the length of the subject */
            0,                    /* start at offset 0 in the subject */
            0,                    /* default options */
            this->regmatch,              /* output vector for substring information */
            sizeof(this->regmatch)/sizeof(this->regmatch[0]));           /* number of elements in the output vector */
    }
};

struct test_posix_search
{
    regex_t rgx;
    bool err;
    regmatch_t regmatch[3];

    test_posix_search(const char * pattern)
    {
        this->err = regcomp(&this->rgx, pattern, REG_EXTENDED);
    }

    ~test_posix_search()
    {
        regfree(&rgx);
    }

    bool check_pre_condition(const char * )
    {
        return !this->err;
    }

    void exec(const char * s)
    {
        regexec(&rgx, s, 1, this->regmatch, 0);
    }
};

struct test_posix_search_capture : test_posix_search
{
    test_posix_search_capture(const char * pattern)
    : test_posix_search(pattern)
    {}

    void exec(const char * s)
    {
        while (0 == regexec(&rgx, s, sizeof(this->regmatch)/sizeof(this->regmatch[0]), this->regmatch, 0)) {
            if (!this->regmatch[0].rm_eo) {
                break;
            }
            s += this->regmatch[0].rm_eo;
        }
    }
};

template<typename BaseTest>
class Bench
{
    struct Test
    : BaseTest
    {
        Test(const char * pattern)
        : BaseTest(pattern)
        , pattern(pattern)
        {}

        const char * pattern;
    };

    struct ref_test
    {
        Test & test;

        ref_test(Test & test)
        : test(test)
        {}

        bool check_pre_condition(const char * s) const
        { return this->test.check_pre_condition(s); }

        void exec(const char * s) const
        { return this->test.exec(s); }
    };

    static void test(Test & test, const char * s)
    {
        ::test(basic_benchmark<ref_test>(test.pattern, ref_test(test)), 600000u, s);
    }

public:
    Bench()
    {
        display_timer timer;

        {
            Test t("a");
            test(t, "aaa");
            test(t, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        }
        {
            Test t("a*");
            test(t, "aaa");
            test(t, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        }
        {
            Test t("a+");
            test(t, "aaa");
            test(t, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        }
        {
            Test t("a?");
            test(t, "aaa");
            test(t, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        }
        {
            Test t(".");
            test(t, "aaa");
            test(t, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        }
        {
            Test t(".*");
            test(t, "aaa");
            test(t, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        }
        {
            Test t(".+");
            test(t, "aaa");
            test(t, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        }
        {
            Test t(".?");
            test(t, "aaa");
            test(t, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        }
        {
            Test t("^a");
            test(t, "aaa");
            test(t, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        }
        {
            Test t("a$");
            test(t, "aaa");
            test(t, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        }
        {
            Test t("^a.*a$");
            test(t, "aaa");
            test(t, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        }
        {
            Test t("\\w");
            test(t, "aaa");
            test(t, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        }
        {
            Test t("\\d");
            test(t, "000");
            test(t, "000000000000000000000000000000000000000000000000000000000000000000");
        }
        {
            Test t("\\s");
            test(t, "   ");
            test(t, "                                                                  ");
        }
        {
            Test t("\\w*");
            test(t, "aaa");
            test(t, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        }
        {
            Test t("\\d*");
            test(t, "000");
            test(t, "000000000000000000000000000000000000000000000000000000000000000000");
        }
        {
            Test t("\\s*");
            test(t, "   ");
            test(t, "                                                                  ");
        }

        {
            Test t("[0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz]");
            test(t, "abc");
            test(t, "jhkgbgJKGJKgk72287428jhnJKHbkuyg277hjbgkhG72Tbhgjkgjbgjhbgj266GBTn");
        }
        {
            Test t("[0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz]+");
            test(t, "abc");
            test(t, "jhkgbgJKGJKgk72287428jhnJKHbkuyg277hjbgkhG72Tbhgjkgjbgjhbgj266GBTn");
        }
        {
            Test t("(?:\\d\\d(?: *|(?:\\.|-)?)){5}");
            test(t, "0123456789");
            test(t, "01      23-45.67                89");
        }
        {
            Test t("(.*):(.*):(.*)");
            test(t, "::");
            test(t, "aaaaaaaaaaaaaaaaaaaa:bbbbbbbbbbbbbbbbbbbb:cccccccccccccccccccc");
        }

        std::cout << "\ntotal: ";
    }
};

int main()
{
    std::cout << "search:\n";
    Bench<test_search>();
    std::cout << "\n\nsearch (optmize_mem=true):\n";
    Bench<test_search_optimize_mem>();
    std::cout << "\n\npcre search:\n";
    Bench<test_pcre_search>();
    std::cout << "\n\nsearch with capture:\n";
    Bench<test_search_capture>();
    std::cout << "\n\nsearch with capture (optmize_mem=true):\n";
    Bench<test_search_capture_optimize_mem>();
    std::cout << "\n\npcre search with capture:\n";
    Bench<test_pcre_search_capture>();
    std::cout << "\n\nposix search:\n";
    Bench<test_posix_search>();
    std::cout << "\n\nposix search with capture:\n";
    Bench<test_posix_search_capture>();
}
