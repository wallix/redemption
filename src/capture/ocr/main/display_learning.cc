#include <mln/image/image2d.hh>
#include <mln/io/pbm/load.hh>

#include "capture/ocr/labelize.hh"
#include "capture/ocr/io_char_box.hpp"

#include <iostream>
#include <cstring>
#include <cerrno>


inline void usage(char** argv)
{
    std::cerr << "Usage: " << argv[0] << " input.pbm" << std::endl;
}


int main(int argc, char** argv)
{
    if (argc != 2) {
        usage(argv);
        return 1;
    }

    mln::image2d<bool> input;
    if (!mln::io::pbm::load(input, argv[1])) {
        if (errno != 0) {
            std::cerr << argv[0] << ": " << strerror(errno) << std::endl;
        }
        return 2;
    }
    display_char_box(std::cout, input, ocr::labelize(input));
    return 0;
}
