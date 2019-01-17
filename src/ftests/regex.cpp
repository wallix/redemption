#include "regex/regex.hpp"

#include <iostream>

// g++ src/ftests/regex.cpp -I src/regex -I src/ -fsanitize=address -g
// g++ src/ftests/regex.cpp -I src/regex -I src/ -fsanitize=address -g -DDISPLAY_TRACE=1

int main(int ac,  char ** av) {
    if (ac < 2) {
        std::cerr << av[0] << " pattern string [0:search, 1:search_with_trace]" << std::endl;
        return 1;
    }
    bool is_search_with_matches = (ac == 4 && av[3][0] == '1');
    char const * pattern = av[1];
    char const * str = av[2];
    char const * original_str = str;

    re::Regex reg(pattern);

    auto search = [&](char const * str){
        std::cout << "str: " << str << "\n";
        bool result_search = is_search_with_matches ? reg.search_with_matches(str) : reg.search(str);
        std::cout <<
            "pattern: " << pattern << "\n"
            "string: " << str << "\n"
            "search: " << result_search << "\n"
            "last_index: " << reg.last_index() << "\n"
            "mod with trace: " << is_search_with_matches << "\n";
        if (is_search_with_matches && result_search) {
            re::Regex::range_matches matches;
            reg.match_result(matches, false);
            std::cout << "  empty: " << matches.empty() << "\n";
            std::cout << "  first: '" << ((!matches.empty() && matches[0].first) ? matches[0].first : "") << "'\n";
        }

        return str + reg.last_index();
    };

    while (*str) {
        str = search(str);
        std::cout << "\n";
    }

    re::Regex::PartOfText searcher = reg.part_of_text_search(false);
    str = original_str;

    for (; *str; ++str) {
        char const c[]{*str, 0, 0, 0, 0};
        auto const r = searcher.next(c);
        switch (r) {
            #define CASE(status) case re::Regex::match_##status: \
                std::cout << *str << ": " #status << std::endl;  \
                break
            CASE(success);
            CASE(fail);
            CASE(undetermined);
            #undef CASE
        }

        if (r == re::Regex::match_success) {
            searcher = reg.part_of_text_search(false);
            search(std::string(original_str, str+1).c_str());
            original_str = str;
        }
    }
}
