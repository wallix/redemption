/*
   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   redrec video verifier program
*/

#include <iostream>

#include <utility>
#include <string>

#include "version.hpp"
#include "crypto_in_filename_transport.hpp"
#include "in_meta_sequence_transport.hpp"
#include "ssl_calls.hpp"
#include "config.hpp"
#include "fdbuf.hpp"

#include "program_options/program_options.hpp"

#ifndef HASH_LEN
#define HASH_LEN 64
#endif

static inline bool check_file_hash_sha256(
    const char * file_path,
    uint8_t const * crypto_key,
    size_t          key_len,
    uint8_t const * hash_buf,
    size_t          hash_len,
    size_t len_to_check
) {
    REDASSERT(SHA256_DIGEST_LENGTH == hash_len);

    SslHMAC_Sha256 hmac(crypto_key, key_len);

    io::posix::fdbuf file;
    file.open(file_path, O_RDONLY);
    if (!file.is_open()) {
        LOG(LOG_ERR, "failed opening=%s", file_path);
        return false;
    }

    uint8_t buf[4096];
    size_t  number_of_bytes_read = 0;
    int len_read = 0;
    do {
        len_read = file.read(buf,
                ((len_to_check == 0) ||(number_of_bytes_read + sizeof(buf) < len_to_check))
                ? sizeof(buf)
                : len_to_check - number_of_bytes_read);
        if (len_read <= 0){
            break;
        }
        hmac.update(buf, static_cast<size_t>(len_read));
        number_of_bytes_read += len_read;
    } while (number_of_bytes_read < len_to_check || len_to_check == 0);

    if (len_read < 0){
        LOG(LOG_ERR, "failed reading=%s", file_path);
        return false;
    }

    file.close();
    uint8_t         hash[SHA256_DIGEST_LENGTH];
    hmac.final(&hash[0], SHA256_DIGEST_LENGTH);

    if (memcmp(hash, hash_buf, hash_len)) {
        LOG(LOG_ERR, "failed checking hash=%s", file_path);
    }

    return (memcmp(hash, hash_buf, hash_len) == 0);
}

bool check_file_hash(
    const char * file_path,
    uint8_t const * crypto_key, size_t key_len,
    const char (&hash)[HASH_LEN],
    bool quick_check
) {
    uint8_t const * hash_ = reinterpret_cast<uint8_t const *>(hash);
    if (quick_check){
        return check_file_hash_sha256(file_path, crypto_key, key_len, hash_, HASH_LEN / 2, 4096);
    }
    return check_file_hash_sha256(file_path, crypto_key, key_len, hash_ + (HASH_LEN / 2), HASH_LEN / 2, 0);
}

bool check_mwrm_file(CryptoContext * cctx, const char * file_path, const char (&hash)[HASH_LEN], bool quick_check) {
    TODO("Add unit test for this function")
    bool result = false;

    if (check_file_hash(file_path, cctx->hmac_key, sizeof(cctx->hmac_key), hash, false) == true) {

        unsigned char derivator[DERIVATOR_LENGTH];
        get_derivator(file_path, derivator, DERIVATOR_LENGTH);
        unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
        if (compute_hmac(trace_key, cctx->crypto_key, derivator) == -1){
            LOG(LOG_ERR, "failed to compute message authentication code");
            return false;
        }

        transbuf::icrypto_filename_buf ifile(cctx);
        if (ifile.open(file_path) < 0) {
            LOG(LOG_ERR, "failed opening=%s", file_path);
            return false;
        }

        struct ReaderBuf
        {
            transbuf::icrypto_filename_buf & buf;

            ssize_t operator()(char * buf, size_t len) const {
                return this->buf.read(buf, len);
            }
        };

        detail::ReaderLine<ReaderBuf> reader({ifile});
        auto meta_header = detail::read_meta_headers(reader);

        detail::MetaLine meta_line;

        result = true;

        while (detail::read_meta_file(reader, meta_header, meta_line) !=
               ERR_TRANSPORT_NO_MORE_DATA) {
            if ((quick_check &&
                 (check_file_hash_sha256(
                          meta_line.filename
                        , cctx->hmac_key
                        , sizeof(cctx->hmac_key)
                        , meta_line.hash1
                        , sizeof(meta_line.hash1)
                        , 4096) == false)) ||
                (!quick_check &&
                 (check_file_hash_sha256(
                          meta_line.filename
                        , cctx->hmac_key
                        , sizeof(cctx->hmac_key)
                        , meta_line.hash2
                        , sizeof(meta_line.hash2)
                        , 0) == false))) {
                result = false;
                break;
            }
        }
    }

    return result;
}

void make_file_path(const char * directory_name, const char * file_name,  char * file_path_buf, size_t file_path_len) {
    snprintf(file_path_buf, file_path_len, "%s%s%s", directory_name,
        ((directory_name[strlen(directory_name) - 1] == '/') ? "" : "/"),
        file_name);

    file_path_buf[file_path_len - 1] = '\0';
}

template<typename T>
int check_encrypted_or_checksumed_file(CryptoContext & cctx,
                                       std::string const & input_filename,
                                       std::string const & hash_path,
                                       const char * fullfilename,
                                       bool quick_check,
                                       uint32_t verbose) {
    /*****************
    * Load file hash *
    *****************/
    char hash[HASH_LEN];

    {
        char file_path[1024] = {};

        size_t filename_len = input_filename.length();

        bool hash_ok = false;

        make_file_path(hash_path.c_str(), input_filename.c_str(), file_path, sizeof(file_path));
        std::cout << "hash file path: \"" << file_path << "\"." << std::endl;

        char   temp_buffer[4096];
        char * buf;

        try {
            T in_hash_t(&cctx, file_path);
            if (verbose) {
                LOG(LOG_INFO, "Transport created");
            }

            buf = temp_buffer;

            memset(temp_buffer, 0, sizeof(temp_buffer));

            const size_t hash_data_length = filename_len + 1 + HASH_LEN;
            if (verbose) {
                LOG(LOG_INFO, "Hash data length=%zu", hash_data_length);
            }
            in_hash_t.recv(&buf, hash_data_length);

            if (verbose) {
                LOG(LOG_INFO, "Hash data received");
            }

            // Filename HASH_64_BYTES
            //         ^
            //         |
            //     separator
            if (!memcmp(temp_buffer, input_filename.c_str(), filename_len) &&
                // Separator
                (temp_buffer[filename_len] == ' ')) {
                if (temp_buffer + filename_len + 1 + HASH_LEN == buf){
                    memcpy(hash, temp_buffer + filename_len + 1, sizeof(hash));
                    hash_ok = true;
                }
                else {
                    std::cerr << "Truncated hash: \"" << file_path << "\"" << std::endl << std::endl;
                }
            }
            else {
                std::cerr << "File name mismatch: \"" << file_path << "\"" << std::endl << std::endl;
            }
        }
        catch (Error const & e) {
            std::cerr << "Exception code (hash): " << e.id << std::endl << std::endl;
        }
        catch (...) {
            std::cerr << "Cannot read hash file: \"" << file_path << "\"" << std::endl << std::endl;
        }

        if (hash_ok == false) {
            exit(-1);
        }
    }

    /******************
    * Check mwrm file *
    ******************/
    if (check_mwrm_file(&cctx, fullfilename, hash, quick_check) == false) {
        std::cerr << "File \"" << fullfilename << "\" is invalid!" << std::endl << std::endl;

        exit(-1);
    }

    std::cout << "No error detected during the data verification." << std::endl << std::endl;

    return 0;
}

template<class F>
int app_verifier(int argc, char ** argv, const char * copyright_notice, F crypto_context_initializer) {
    static_assert(
        std::is_same<int, decltype(crypto_context_initializer(std::declval<CryptoContext&>()))>::value
      , "crypto_context_initializer result type may be 'int'"
    );

    openlog("verifier", LOG_CONS | LOG_PERROR, LOG_USER);

    Inifile ini;
    ConfigurationLoader cfg_loader(ini, CFG_PATH "/" RDPPROXY_INI);

    std::string hash_path      = ini.get<cfg::video::hash_path>().c_str()  ;
    std::string mwrm_path      = ini.get<cfg::video::record_path>().c_str();
    std::string input_filename                                ;
    bool        quick_check    = false                        ;
    uint32_t    verbose        = 0                            ;

    program_options::options_description desc({
        {'h', "help",    "produce help message"},
        {'v', "version", "show software version"},
        {'q', "quick",   "quick check only"},
        {'s', "hash-path",  &hash_path,         "hash file path"       },
        {'m', "mwrm-path",  &mwrm_path,         "mwrm file path"       },
        {'i', "input-file", &input_filename,    "input mwrm file name" },
        {"verbose",         &verbose,           "more logs"            },
    })
    ;

    auto options = program_options::parse_command_line(argc, argv, desc);

    if (options.count("help") > 0) {
        std::cout << copyright_notice;
        std::cout << "Usage: redver [options]\n\n";
        std::cout << desc << std::endl;
        exit(0);
    }

    if (options.count("version") > 0) {
        std::cout << copyright_notice;
        exit(0);
    }

    if (options.count("quick") > 0) {
        quick_check = true;
    }

    if (hash_path.c_str()[0] == 0) {
        std::cerr << "Missing hash-path : use -h path\n\n";
        exit(-1);
    }

    if (mwrm_path.c_str()[0] == 0) {
        std::cerr << "Missing mwrm-path : use -m path\n\n";
        exit(-1);
    }

    if (input_filename.c_str()[0] == 0) {
        std::cerr << "Missing input mwrm file name : use -i filename\n\n";
        exit(-1);
    }

    {
        char temp_path[1024]     = {};
        char temp_basename[1024] = {};
        char temp_extension[256] = {};

        canonical_path(input_filename.c_str(), temp_path, sizeof(temp_path), temp_basename, sizeof(temp_basename), temp_extension, sizeof(temp_extension), verbose);
        //cout << "temp_path: \"" << temp_path << "\", \"" << temp_basename << "\", \"" << temp_extension << "\"" << std::endl;

        if (strlen(temp_path) > 0) {
            mwrm_path       = temp_path;
            input_filename  = temp_basename;
            input_filename += temp_extension;
        }
    }


    char fullfilename[2048];

    make_file_path(mwrm_path.c_str(), input_filename.c_str(), fullfilename, sizeof(fullfilename));

    std::cout << "Input file is \"" << fullfilename << "\".\n";


    if (verbose) {
        LOG(LOG_INFO, "hash_path=\"%s\"", hash_path.c_str());
        LOG(LOG_INFO, "mwrm_path=\"%s\"", mwrm_path.c_str());
        LOG(LOG_INFO, "file_name=\"%s\"", input_filename.c_str());
    }

    bool infile_is_encrypted = false;

    if (auto file = io::posix::fdbuf(open(fullfilename, O_RDONLY)))
    {
        uint32_t magic_test;
        TODO("No portable code endianess")
        ssize_t res_test = file.read(&magic_test, sizeof(magic_test));
        if ((res_test == sizeof(magic_test)) &&
            (magic_test == WABCRYPTOFILE_MAGIC)) {
            infile_is_encrypted = true;
            std::cout << "Input file is encrypted.\n";
        }
    }
    else {
        std::cerr << "Input file is absent.\n";
        exit(-1);
    }

    unsigned infile_version = 1;
    bool infile_is_checksumed = false;

    if (!infile_is_encrypted) {
        transbuf::ifile_buf ifile;
        ifile.open(fullfilename);

        struct ReaderBuf
        {
            transbuf::ifile_buf & buf;

            ssize_t operator()(char * buf, size_t len) const {
                return this->buf.read(buf, len);
            }
        };

        detail::ReaderLine<ReaderBuf> reader({ifile});
        auto meta_header = detail::read_meta_headers(reader);

        infile_version       = meta_header.version;
        infile_is_checksumed = meta_header.has_checksum;
    }

    if (!infile_is_encrypted && ((infile_version < 2) || !infile_is_checksumed)) {
        std::cout << "Input file is unencrypted.\n";
        return 0;
    }

    CryptoContext cctx;
    memset(&cctx, 0, sizeof(cctx));
    if (int status = crypto_context_initializer(cctx)) {
        return status;
    }

    OpenSSL_add_all_digests();

    return check_encrypted_or_checksumed_file<CryptoInFilenameTransport>(cctx, input_filename,
        hash_path, fullfilename, quick_check, verbose);
}
