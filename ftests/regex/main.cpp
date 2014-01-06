#include <iostream>
#include <ctime>
#include <sys/types.h>
#include <regex.h>

#include "regex.hpp"

struct Tracer {
    void start(unsigned idx) const
    {
        std::cout << ("start:\tidx: ") << idx << "\n";
    }

    void new_id(unsigned old_id, unsigned new_id) const
    {
        std::cout << ("new_id:\told: ") << old_id << "\tnew: " << new_id << "\n";
    }

    bool open(unsigned idx, const char * s, unsigned num_cap) const
    {
        std::cout << "open:\tidx: " << idx << "\tc: " << *s << "\tcap: " << num_cap << "\n";
        return true;
    }

    bool close(unsigned idx, const char * s, unsigned num_cap) const
    {
        std::cout << "close:\tidx: " << idx << "\tc: " << *s << "\tcap: " << num_cap << "\n";
        return true;
    }

    void fail(unsigned idx) const
    {
        std::cout << ("fail:\tidx: ") << idx << "\n";
    }

    void good(unsigned idx) const
    {
        std::cout << ("good:\tidx: ") << idx << "\n";
    }
};

int main(int argc, char **argv) {
    //std::ios::sync_with_stdio(false);

    using namespace re;

    if (argc < 2) {
        std::cerr << argv[0] << (" regex [str [optimization:(1|2|3)]]") << std::endl;
        return 1;
    }
    const int optimize_mem = argc >= 4 && argv[3] ? atoi(argv[3]) : 0;
    std::cout << "optimize_mem: " << optimize_mem << "\n";
    const char * rgxstr = argv[1];
    Regex regex(argv[1],
                optimize_mem == 1
                    ? Regex::OPTIMIZE_MEMORY
                : optimize_mem == 2
                    ? Regex::MINIMAL_MEMORY
                : optimize_mem > 2
                    ? Regex::OPTIMIZE_MEMORY|Regex::MINIMAL_MEMORY
                : Regex::DEFAULT_FLAG
               );
    if (regex.message_error()) {
        std::cerr << regex.message_error() << " at position " << regex.position_error() << std::endl;
        return 2;
    }
    regex.display();
    std::cout.flush();

    if (argc < 3) {
        std::string line;
        std::cout << "str: ";
        std::getline(std::cin, line);
        Regex::ExactPartOfText part_rgx = regex.part_of_text_exact_search(line.empty());
        if (Regex::match_undetermined == part_rgx.state()) {
            while (Regex::match_undetermined == part_rgx.next(line.c_str())
                && (std::cout << "str: ", std::getline(std::cin, line))) {
            }
        }

        switch (part_rgx.state()) {
            case Regex::match_success: std::cout << "ok\n"; return 0;
            case Regex::match_fail: std::cout << "fail\n"; return 1;
            default: ;
        }

        if (Regex::match_success == part_rgx.finish()) {
            std::cout << "ok\n";
            return 0;
        }
        std::cout << "fail\n";
        return 1;
    }
    else {
        argv[1] = argv[2];
    }


    //display_state(st);

    regex_t rgx;
    if (0 != regcomp(&rgx, rgxstr, REG_EXTENDED)) {
        std::cout << ("regcomp error") << std::endl;
        regcomp(&rgx, "", REG_EXTENDED);
    }
    regmatch_t regmatch[3];

    bool ismatch1 = false;
    bool ismatch2 = false;
    bool ismatch3 = false;
    bool ismatch4 = false;
    double d1, d2, d3, d4;
    bool validregexec = false;

    const char * str = argc > 1 ? argv[1] : "abcdef";

#ifndef ITERATION
# define ITERATION 100000
#endif
    {
        validregexec = regexec(&rgx, str, 1, regmatch, 0) == 0; //NOTE preload
        //BEGIN
        std::clock_t start_time = std::clock();
        for (size_t i = 0; i < ITERATION; ++i) {
            ismatch1 = 0 == regexec(&rgx, str, 1, regmatch, 0);
        }
        d1 = double(std::clock() - start_time) / CLOCKS_PER_SEC;
        //END
    }
    {
#ifdef DISPLAY_TRACE
        std::cout << ("\n### search\n") << std::endl;
#endif
        std::clock_t start_time = std::clock();
        for (size_t i = 0; i < ITERATION; ++i) {
            ismatch2 = regex./*exact_*/search(str);
        }
        d2 = double(std::clock() - start_time) / CLOCKS_PER_SEC;
    }
    //std::streambuf * dbuf = std::cout.rdbuf(0);
    {
        struct test {
            inline static bool
            impl(regex_t& rgx, const char * s, regmatch_t * m, unsigned size) {
                const char * str = s;
                while (0 == regexec(&rgx, s, size, m, 0)) {
                    if (!m[0].rm_eo) {
                        break;
                    }
                    for (unsigned i = 1; i < size; i++) {
                        if (m[i].rm_so == -1) {
                            break;
                        }
                        int start = m[i].rm_so + (s - str);
                        int finish = m[i].rm_eo + (s - str);
                        (void)start;
                        (void)finish;
                        //std::cout.write(str+start, finish-start) << "\n";
                    }
                    s += m[0].rm_eo;
                }
                return 0 == *s;
            }
        };
        test::impl(rgx, str, regmatch, sizeof(regmatch)/sizeof(regmatch[0])); //NOTE preload
        //BEGIN
        std::clock_t start_time = std::clock();
        for (size_t i = 0; i < ITERATION; ++i) {
            ismatch3 = test::impl(rgx, str, regmatch, sizeof(regmatch)/sizeof(regmatch[0]));
        }
        d3 = double(std::clock() - start_time) / CLOCKS_PER_SEC;
        //END
    }
    {
#ifdef DISPLAY_TRACE
        std::cout << ("\n### exact_search_with_matches\n") << std::endl;
#endif
        std::clock_t start_time = std::clock();
        for (size_t i = 0; i < ITERATION; ++i) {
            if ((ismatch4 = regex.exact_search_with_matches(str))) {
                Regex::range_matches match_result = regex.match_result();
                typedef Regex::range_matches::iterator iterator;
                for (iterator first = match_result.begin(), last = match_result.end(); first != last; ++first) {
                    //std::cout.write(str+first->first, first->second) << "\n";
                }
            }
        }
        d4 = double(std::clock() - start_time) / CLOCKS_PER_SEC;
    }
    //std::cout.rdbuf(dbuf);

    std::cout.precision(2);
    std::cout.setf(std::ios::fixed);
    std::cout
    << "regex: " << rgxstr << "\n"
    << "search:\n"
    << (ismatch1 ? "good\n" : "fail\n")
    << d1 << " sec\n"
    << (ismatch2 ? "good\n" : "fail\n")
    << d2 << " sec\n"
    << "match:\n"
    << (ismatch3 ? "good\n" : "fail\n")
    << d3 << " sec\n"
    << (ismatch4 ? "good\n" : "fail\n")
    << d4 << " sec\n";

    if (validregexec && ismatch3) {
        std::cout << ("with regex.h\n");
        for (unsigned i = 1; i < sizeof(regmatch)/sizeof(regmatch[0]); i++) {
            if (regmatch[i].rm_so == -1) {
                break;
            }
            int start = regmatch[i].rm_so;
            int finish = regmatch[i].rm_eo;
            (std::cout << "\tmatch: '").write(str+start, finish-start) << "'\n";
        }
    }
    if (ismatch4) {
        std::cout << ("with dfa\n");
        Regex::range_matches match_result = regex.match_result();
        typedef Regex::range_matches::iterator iterator;
        for (iterator first = match_result.begin(), last = match_result.end(); first != last; ++first) {
            std::cout << (first->first - str) << " " << (first->second - str) << std::endl;
            if (first->first) {
                (std::cout << "\tmatch: '").write(first->first, first->second-first->first) << "'\n";
            }
        }
        std::cout.flush();
    }
    regfree(&rgx);
}
