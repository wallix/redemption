/*
   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   redrec video converter program
*/

#include <iostream>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <utility>
#include <string>

#include "version.hpp"

#include "capture.hpp"
#include "FileToGraphic.hpp"
#include "crypto_in_filename_transport.hpp"
#include "out_file_transport.hpp"

#ifndef HASH_LEN
#define HASH_LEN 64
#endif

int main(int argc, char ** argv) {
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
        cout << copyright_notice;
        cout << "Usage: redrec [options]\n\n";
        cout << desc << endl;
        exit(-1);
    }

    if (options.count("version") > 0) {
        cout << copyright_notice;
        exit(-1);
    }

    if (input_filename.c_str()[0] == 0) {
        cout << "Missing input filename : use -i filename\n\n";
        exit(-1);
    }

    if (output_filename.c_str()[0] == 0) {
        cout << "Missing output filename : use -o filename\n\n";
        exit(-1);
    }


    bool infile_is_encrypted = false;

    {
        int      fd_test;
        uint32_t magic_test;
        int      res_test;

        fd_test = open(input_filename.c_str(), O_RDONLY);
        if (fd_test != -1) {
            // Reads file header (4 bytes).
            res_test = read(fd_test, &magic_test, sizeof(magic_test));
            if ((res_test == sizeof(magic_test)) &&
                (magic_test == WABCRYPTOFILE_MAGIC)) {
                infile_is_encrypted = true;
            }
            close(fd_test);
        }
    }

    if (infile_is_encrypted == false) {
        cout << "Input file is unencrypted.\n";
        exit(0);
    }

    Inifile ini;

    CryptoContext cctx;
    memset(&cctx, 0, sizeof(cctx));
    memcpy(cctx.crypto_key, ini.crypto.key0, sizeof(cctx.crypto_key));
    memcpy(cctx.hmac_key,   ini.crypto.key1, sizeof(cctx.hmac_key  ));
    OpenSSL_add_all_digests();

    CryptoInFilenameTransport in_t(&cctx, input_filename.c_str());

    int fd = -1;

    try {
        int fd = open(output_filename.c_str(), O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR);

        if (fd != -1) {
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
            catch (Error e) {
                if (e.id != ERR_TRANSPORT_NO_MORE_DATA) {
                    throw;
                }

                out_t.send(mem, buf - mem);
            }

            close(fd);
        }
    }
    catch (...) {
        if (fd != -1) {
            close(fd);
        }

        throw;
    }

    return 0;
}
