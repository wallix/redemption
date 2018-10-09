#include <mln/image/image2d.hh>
#include <mln/io/pbm/load.hh>

#include "labelize.hh"

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cerrno>

inline void usage(char** argv)
{
    std::cerr <<
        "Usage: " << argv[0] << " input.pbm input.txt output.txt\n"
        "  input.pbm = barre de titre\n"
        "  input.txt = caractères de la barre de titre" << std::endl;
}


int main(int argc, char** argv)
{
    if (argc != 4) {
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

    std::vector<ocr::label_attr_t> results = ocr::labelize(input);

    std::ifstream text(argv[2]);
    std::ofstream output(argv[3], std::ios_base::app);

    std::string c;
    typedef std::vector<ocr::label_attr_t>::iterator iterator;
    for (iterator first = results.begin(), last = results.end(); first != last; ++first) {
        if (!(text >> c)) {
            std::cerr << "Error: characters not found." << std::endl;
            return 1;
        }
        output << c << "\t"
          << first->area << "\t"
          << first->bbox.ncols() << "\t"
          << first->bbox.nrows() << "\t";

        int j = first->bbox.nsites();
        mln::image2d<bool>::piter p(first->bbox);
        for (p.start(); p.is_valid(); p.next()) {
            output << input(p) << ((j-- > 1) ? "\t" : "\n");
        }
    }

    if (text >> c) {
        std::cerr << "Error: box not found." << std::endl;
        return 2;
    }

    return 0;
}
