#include <boost/xpressive/xpressive.hpp>
// include <boost/regex.hpp>
#include <boost/function.hpp>
#include <iostream>
#include <vector>

void boost_regex_test(const char * str_text, const char * rgx_text,
                      std::string& vs, uint iteration = 1)
{
    uint count = 0;
// 	boost::regex rgx(rgx_text);
    typedef boost::xpressive::cregex cregex;
    cregex rgx = cregex::compile(rgx_text, rgx_text + strlen(rgx_text));
    using namespace boost::xpressive;
// 	cregex rgx = (s1=(*~(set='{','}') >> (set='{','}')));

    for (uint i = 0; i < iteration; ++i) {
        vs.clear();
        typedef const char * const_iterator;
        const_iterator start = str_text;
        const_iterator end = str_text + strlen(str_text);
        boost::xpressive::match_results<const_iterator> what;
        boost::xpressive::regex_constants::match_flag_type flags = boost::xpressive::regex_constants::match_default;
        while (boost::xpressive::regex_search(
            start, end,
            what,
            rgx,
            flags
        )){
            std::string str = what.str();
            char c = str[str.size()-1];
            for (uint n = 0; n < count; ++n) {
                vs += '\t';
            }
            if (c == '{') {
                ++count;
                vs += str;
            }
            else {
                if (!count--){
                    throw std::runtime_error("invalid sequence");
                    count = 1;
                }
                vs += str.substr(0, str.size()-1);
                vs += '\n';
                for (uint n = 0; n < count; ++n) {
                    vs += '\t';
                }
                vs += '}';
            }
            vs += '\n';
            start = what[0].second;
            // update flags:
// 			flags |= boost::xpressive::regex_constants::match_prev_avail|boost::xpressive::regex_constants::match_not_bob;
        }
    }
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <iostream>
#include <string>
#include <stdexcept>

/* The following is the size of a buffer to contain any error messages
*  encountered when the regular expression is compiled. */

#define MAX_ERROR_MSG 0x1000

/* Compile the regular expression described by "regex_text" into
*  "r". */

static int compile_regex (regex_t * r, const char * regex_text)
{
    int status = regcomp (r, regex_text, REG_EXTENDED|REG_NEWLINE);
    if (status != 0) {
        char error_message[MAX_ERROR_MSG];
        regerror (status, r, error_message, MAX_ERROR_MSG);
        printf ("Regex error compiling '%s': %s\n",
                        regex_text, error_message);
        return 1;
    }
    return 0;
}

/*
* Match the string in "to_match" against the compiled regular
* expression in "r".
*/

static int match_regex (regex_t * r, const char * to_match, std::string& vs)
{
    uint count = 0;
    /* "P" is a pointer into the string which points to the end of the
    *      previous match. */
    const char * p = to_match;
    /* "N_matches" is the maximum number of matches allowed. */
    const int n_matches = 1;
    /* "M" contains the matches found. */
    regmatch_t m[1];

    int nomatch = 0;

    while (!(nomatch = regexec (r, p, n_matches, m, 0))) {
        for (int i = 0; i < n_matches; i++) {
            if (m[i].rm_so == -1) {
                break;
            }
            int start = m[i].rm_so + (p - to_match);
            int finish = m[i].rm_eo + (p - to_match);
// 			printf ("$& is ");
// 			printf ("'%.*s' (bytes %d:%d)\n", (finish - start),
// 							to_match + start, start, finish);

            for (uint n = 0; n < count; ++n) {
                vs += '\t';
            }
            if (*(to_match + finish - 1) == '{') {
                ++count;
                vs.insert(vs.size(), to_match + start, finish - start);
            }
            else {
                if (!count--){
                    throw std::runtime_error("invalid sequence");
                }
                vs.insert(vs.size(), to_match + start, finish - start - 1);
                vs += '\n';
                for (uint n = 0; n < count; ++n) {
                    vs += '\t';
                }
                vs += '}';
            }
            vs += '\n';
        }
        p += m[0].rm_eo;
    }
    return nomatch;
}

void posix_test(const char * str, const char * rgx,
                            std::string& vs, uint iteration = 1)
{
    regex_t r;
    struct _ {
        regex_t& reg;
        _(regex_t& r	):reg(r) {}
        ~_() {regfree(& reg);}
    } _(r);
    compile_regex(& r, rgx);
    for (uint i = 0; i < iteration; ++i) {
        vs.clear();
        match_regex(& r, str, vs);
    }
}

int main(int argc, char ** argv)
{
    if (argc < 2) {
        std::cout << argv[0] << "posix|boost [iteration] [text]\n";
    }
    uint iteration = argc >= 3 ? atoi(argv[2]) : 1;
    const char * text = argc == 4 ? argv[3] : "a{b{c{d}}e}";
    const char * regex = "([^\\{|\\}]*[\\{|\\}])";
    std::string vs;
    if (argc > 1 && argv[1][0] == 'p')
        posix_test(text, regex, vs, iteration);
    else
        boost_regex_test(text, regex, vs, iteration);
    std::cout << vs << "\n";
    return 0;
}
