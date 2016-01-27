/*
   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   redrec video converter program
*/

#ifndef REDEMPTION_UTILS_APPS_APP_DECRYPTER_HPP
#define REDEMPTION_UTILS_APPS_APP_DECRYPTER_HPP

#include <type_traits>
#include <iostream>
#include <cstring>
#include <string>
#include <cerrno>

#include "transport/in_filename_transport.hpp"
#include "transport/out_file_transport.hpp"
#include "fdbuf.hpp"

#include "program_options/program_options.hpp"


int app_decrypter(int argc, char ** argv, const char * copyright_notice, CryptoContext & cctx)
{
    openlog("decrypter", LOG_CONS | LOG_PERROR, LOG_USER);

    std::string input_filename;
    std::string output_filename;

    uint32_t verbose = 0;

    program_options::options_description desc({
        {'h', "help",    "produce help message"},
        {'v', "version", "show software version"},

        {'o', "output-file", &output_filename, "output base filename"},
        {'i', "input-file",  &input_filename,  "input base filename"},
        {"verbose",          &verbose,         "more logs"}
    });

    auto options = program_options::parse_command_line(argc, argv, desc);

    if (options.count("help") > 0) {
        std::cout << copyright_notice << "\n\n";
        std::cout << "Usage: redrec [options]\n\n";
        std::cout << desc << std::endl;
        return 0;
    }

    if (options.count("version") > 0) {
        std::cout << copyright_notice << std::endl << std::endl;
        return 0;
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

    if (io::posix::fdbuf file {open(input_filename.c_str(), O_RDONLY)}) {
        uint32_t magic_test;
        // Reads file header (4 bytes).
        int res_test = file.read(&magic_test, sizeof(magic_test));
        if ((res_test == sizeof(magic_test)) &&
            (magic_test == WABCRYPTOFILE_MAGIC)) {
            infile_is_encrypted = true;
        }
    }
    else {
        std::cerr << "Input file is absent.\n\n";
        return -1;
    }

    if (infile_is_encrypted == false) {
        std::cout << "Input file is unencrypted.\n\n";
        return 0;
    }

    OpenSSL_add_all_digests();

    InFilenameTransport in_t(&cctx, input_filename.c_str(), 1);

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
                std::cerr << "Exception code: " << e.id << std::endl << std::endl;
            }
            else {
                out_t.send(mem, buf - mem);
            }
        }

        close(fd);
    }
    else {
        std::cerr << strerror(errno) << std::endl << std::endl;
    }

    return 0;
}

#endif
