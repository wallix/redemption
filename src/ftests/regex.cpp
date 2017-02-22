#include "regex.hpp"

#include <iostream>

int main(int ac,  char ** av) {
    if (ac < 2) {
        std::cerr << av[0] << " pattern string" << std::endl;
        return 1;
    }

    re::Regex reg(av[1]);
    bool result_search = reg.search(av[2]);
    std::cout <<
      "pattern: " << av[1] << "\n"
      "string: " << av[2] << "\n"
      "search: " << result_search << "\n"
      "last_index: " << reg.last_index() << std::endl;
}
