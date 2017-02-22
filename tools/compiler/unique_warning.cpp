#include <fstream>
#include <iostream>
#include <string>
#ifdef NO_BOOST
#include <unordered_set>
typedef std::unordered_set<std::string> set_type;
#else
#include <boost/container/flat_set.hpp>
typedef boost::container::flat_set<std::string> set_type;
#endif

void filter(std::istream & is) {
    std::string l;
    std::string block;
    set_type blocks;
    bool cap = false;
    while (std::getline(is, l)) {
        if (l.find('^') != std::string::npos) {
            block += '\n';
            block += l;
            cap = false;
        }
        else if (l.find("warning:") != std::string::npos || l.find("error:") != std::string::npos) {
            if (!block.empty()) {
                blocks.insert(std::move(block));
            }
            block = std::move(l);
            cap = true;
        }
        else if (l.find("note:") != std::string::npos) {
            block += '\n';
            block += l;
            cap = true;
        }
        else if (!block.empty()) {
            if (cap) {
                block += '\n';
                block += l;
            }
            else {
                blocks.insert(std::move(block));
            }
        }
    }

    if (!block.empty()) {
        blocks.insert(std::move(block));
    }

    for (auto & s: blocks) {
        std::cout << s << "\033[00m\n---------------\n";
    }
    std::cout << blocks.size() << " blocks" << std::endl;
}

int main(int ac, char ** av)
{
    if (ac != 2) {
        filter(std::cin);
    }
    else {
        std::ifstream f(av[1]);
        if (f) {
            filter(f);
        }
        else {
            std::cerr << av[0] << ": open " << av[1] << " failed.\n";
        }
    }
}
