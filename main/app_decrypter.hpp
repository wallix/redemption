/*
   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   redrec video converter program
*/

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

#include <iostream>
#include <cstring>
#include <string>
#include <cerrno>

#include "version.hpp"

#include "crypto_in_filename_transport.hpp"
#include "out_file_transport.hpp"
#include "fdbuf.hpp"

#ifndef HASH_LEN
#define HASH_LEN 64
#endif

template<class F>
// crypto_context_initializer = int(CryptoContext&)
int app_decrypter(int argc, char ** argv, F crypto_context_initializer)
{
    static_assert(
        std::is_same<int, decltype(crypto_context_initializer(std::declval<CryptoContext&>()))>::value
      , "crypto_context_initializer result type may be 'int'"
    );

    openlog("decrypter", LOG_CONS | LOG_PERROR, LOG_USER);

    const char * copyright_notice =
        "\n"
        "ReDemPtion DECrypter " VERSION ".\n"
        "Copyright (C) Wallix 2010-2013.\n"
        "Christophe Grosjean, Raphael Zhou.\n"
        "\n"
        ;

    std::string input_filename;
    std::string output_filename;

    uint32_t verbose = 0;

    boost::program_options::options_description desc("Options");
    desc.add_options()
    ("help,h",    "produce help message")
    ("version,v", "show software version")

    ("output-file,o", boost::program_options::value(&output_filename),   "output base filename")
    ("input-file,i",  boost::program_options::value(&input_filename),    "input base filename" )
    ("verbose",       boost::program_options::value<uint32_t>(&verbose), "more logs"           )
    ;

    boost::program_options::variables_map options;
    boost::program_options::store(
        boost::program_options::command_line_parser(argc, argv).options(desc).run(),
        options
    );
    boost::program_options::notify(options);

    if (options.count("help") > 0) {
        std::cout << copyright_notice;
        std::cout << "Usage: redrec [options]\n\n";
        std::cout << desc << std::endl;
        return -1;
    }

    if (options.count("version") > 0) {
        std::cout << copyright_notice;
        return -1;
    }

    if (input_filename.empty()) {
        std::cerr << "Missing input filename : use -i filename\n\n";
        return -1;
    }

    if (output_filename.empty()) {
        std::cerr << "Missing output filename : use -o filename\n\n";
        return -1;
    }


    bool infile_is_encrypted = false;

    if (io::posix::fdbuf file = open(input_filename.c_str(), O_RDONLY)) {
        uint32_t magic_test;
        // Reads file header (4 bytes).
        int res_test = file.read(&magic_test, sizeof(magic_test));
        if ((res_test == sizeof(magic_test)) &&
            (magic_test == WABCRYPTOFILE_MAGIC)) {
            infile_is_encrypted = true;
        }
    }

    if (infile_is_encrypted == false) {
        std::cout << "Input file is unencrypted.\n";
        return 0;
    }

    CryptoContext cctx;
    memset(&cctx, 0, sizeof(cctx));
    if (int status = crypto_context_initializer(cctx)) {
        return status;
    }

    OpenSSL_add_all_digests();

    CryptoInFilenameTransport in_t(&cctx, input_filename.c_str());

    const int fd = open(output_filename.c_str(), O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR);
    if (fd != -1) {
        io::posix::fdbuf file(fd); // auto-close
        OutFileTransport out_t(fd);

        char mem[4096];
        char *buf;

        try {
            while (1) {
                buf = mem;
                in_t.recv(&buf, sizeof(mem));

                out_t.send(mem, sizeof(mem));
            }
        }
        catch (Error const & e) {
            if (e.id != ERR_TRANSPORT_NO_MORE_DATA) {
                throw;
            }

            out_t.send(mem, buf - mem);
        }
    }
    else {
        std::cerr << strerror(errno) << std::endl;
    }

    return 0;
}
