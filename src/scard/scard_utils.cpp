#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>

#include "utils/sugar/bytes_view.hpp"

#include "scard/scard_utils.hpp"


///////////////////////////////////////////////////////////////////////////////



void check_predicate(bool predicate, const char */*file*/, const char *function,
    int line, const char *name, error_type eid)
{
    if (!predicate)
    {
        LOG(LOG_ERR, "%s(%d): unverified predicate '%s'",
            function, line, name);

        throw Error(eid);
    }
}

std::string bytes_to_hex_string(bytes_view data,
    std::string_view byte_separator)
{
    std::stringstream stream;

    stream << std::hex;
    for (auto it = data.begin(); it != data.end(); ++it)
    {
        stream
            << std::setw(2) << std::setfill('0')
            << std::uppercase
            << static_cast<int>(*it);

        if (std::next(it) != data.end())
        {
            stream
                << byte_separator;
        }
    }

    return stream.str();
}

std::string trim(const std::string& string)
{
    const std::size_t first = string.find_first_not_of(' ');

    // return the original string in case it does not contain any space
    if (first == std::string::npos)
    {
        return string;
    }

    const std::size_t last = string.find_last_not_of(' ');

    return string.substr(first, (last - first + 1));
}

bool compare_case_insensitive(
    std::string_view string_1,
    std::string_view string_2)
{
    bool result(false);

    if (string_1.size() == string_2.size())
    {
        result = std::equal(string_1.begin(), string_1.end(), string_2.begin(),
            [](char char_1, char char_2)
            {
                return (std::tolower(char_1) == std::tolower(char_2));
            }
        );
    }

    return result;
}