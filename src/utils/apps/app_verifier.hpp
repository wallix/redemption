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
#include "detail/meta_opener.hpp"
#include "detail/meta_hash.hpp"

#include "program_options/program_options.hpp"

#ifndef HASH_LEN
#define HASH_LEN 64
#endif

#define QUICK_CHECK_LENGTH 4096

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

static inline bool check_file(const char * file_path, bool is_checksumed,
        uint8_t const * crypto_key, size_t key_len, size_t len_to_check,
        bool is_status_enabled, detail::MetaLine const & meta_line) {
    struct stat64 sb;
    memset(&sb, 0, sizeof(sb));
    if (is_status_enabled) {
        lstat64(file_path, &sb);
    }

    bool is_checksum_ok = false;
    if (is_checksumed) {
        is_checksum_ok = check_file_hash_sha256(file_path, crypto_key,
            key_len, (len_to_check ? meta_line.hash1 : meta_line.hash2),
            (len_to_check ? sizeof(meta_line.hash1) : sizeof(meta_line.hash2)),
            len_to_check);
    }

    if (is_checksumed && is_status_enabled) {
        return (is_checksum_ok && (meta_line.size == sb.st_size));
    }

    if (is_checksumed) {
        return is_checksum_ok;
    }

    if (is_status_enabled) {
        return (
                (meta_line.dev   == sb.st_dev  ) &&
                (meta_line.ino   == sb.st_ino  ) &&
                (meta_line.mode  == sb.st_mode ) &&
                (meta_line.uid   == sb.st_uid  ) &&
                (meta_line.gid   == sb.st_gid  ) &&
                (meta_line.size  == sb.st_size ) &&
                (meta_line.mtime == sb.st_mtime) &&
                (meta_line.ctime == sb.st_ctime)
            );
    }

    return true;
}

static inline bool check_file(const char * file_path, bool is_status_enabled,
        detail::MetaLine const & meta_line, size_t len_to_check,
        CryptoContext * cctx) {
    const bool is_checksumed = true;
    return check_file(file_path, is_checksumed, cctx->hmac_key,
        sizeof(cctx->hmac_key), len_to_check, is_status_enabled, meta_line);
}

static inline bool check_file(const char * file_path, bool is_status_enabled,
        detail::MetaLine const & meta_line, size_t len_to_check) {
    const bool      is_checksumed = false;
    const uint8_t * crypto_key    = nullptr;
    size_t          key_len       = 0;
    return check_file(file_path, is_checksumed, crypto_key, key_len,
        len_to_check, is_status_enabled, meta_line);
}

template<typename T, typename ... Args>
bool check_mwrm_file(const char * file_path, bool is_status_enabled,
        detail::MetaLine const & meta_line_mwrm, size_t len_to_check,
        Args ... args) {
    TODO("Add unit test for this function")
    bool result = false;

    if (check_file(file_path, is_status_enabled, meta_line_mwrm, len_to_check,
                   args ...) == true) {
        T ifile(args ...);
        if (ifile.open(file_path) < 0) {
            LOG(LOG_ERR, "failed opening=%s", file_path);
            return false;
        }

        struct ReaderBuf
        {
            T & buf;

            ssize_t operator()(char * buf, size_t len) const {
                return this->buf.read(buf, len);
            }
        };

        detail::ReaderLine<ReaderBuf> reader({ifile});
        auto meta_header = detail::read_meta_headers(reader);

        detail::MetaLine meta_line_wrm;

        result = true;

        while (detail::read_meta_file(reader, meta_header, meta_line_wrm) !=
               ERR_TRANSPORT_NO_MORE_DATA) {
            if (check_file(meta_line_wrm.filename, is_status_enabled,
                           meta_line_wrm, len_to_check, args ...) == false) {
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

template<typename T, typename ... Args>
int check_encrypted_or_checksumed_file(std::string const & input_filename,
                                       std::string const & hash_path,
                                       const char * fullfilename,
                                       bool quick_check,
                                       uint32_t verbose,
                                       Args ... args) {
    unsigned infile_version = 1;
    bool infile_is_checksumed = false;

    {
        T ifile(args ...);
        ifile.open(fullfilename);

        struct ReaderBuf
        {
            T & buf;

            ssize_t operator()(char * buf, size_t len) const {
                return this->buf.read(buf, len);
            }
        };

        detail::ReaderLine<ReaderBuf> reader({ifile});
        auto meta_header = detail::read_meta_headers(reader);

        infile_version       = meta_header.version;
        infile_is_checksumed = meta_header.has_checksum;
    }

    if (verbose) {
        LOG(LOG_INFO, "file_version=%d is_checksumed=%s", infile_version,
            (infile_is_checksumed ? "yes" : "no"));
    }

    static_assert((std::is_same<T, transbuf::icrypto_filename_buf>::value ||
        std::is_same<T, transbuf::ifile_buf>::value), "Unexpected file_buf class!");
    if (std::is_same<T, transbuf::ifile_buf>::value && (infile_version < 2)) {
        std::cout << "Input file is unencrypted.\n";
        return 0;
    }

    /*****************
    * Load file hash *
    *****************/

    detail::MetaLine hash_line = {{}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {}, {}};

    {
        char file_path[PATH_MAX + 1] = {};

        ssize_t filename_len = input_filename.length();

        bool hash_ok = false;

        make_file_path(hash_path.c_str(), input_filename.c_str(), file_path, sizeof(file_path));
        std::cout << "hash file path: \"" << file_path << "\"." << std::endl;

        try {
            T in_hash_fb(args ...);
            in_hash_fb.open(file_path);
            if (verbose) {
                LOG(LOG_INFO, "File buffer created");
            }

            char temp_buffer[8192];
            memset(temp_buffer, 0, sizeof(temp_buffer));

            ssize_t number_of_bytes_read = in_hash_fb.read(temp_buffer, sizeof(temp_buffer));
            if (verbose) {
                LOG(LOG_INFO, "Hash data received. Length=%zd", number_of_bytes_read);
            }

            if (infile_version == 1) {
                if (verbose) {
                    LOG(LOG_INFO, "Hash data v1");
                }

                // Filename HASH_64_BYTES
                //         ^
                //         |
                //     separator
                if (!memcmp(temp_buffer, input_filename.c_str(), filename_len) &&
                    // Separator
                    (temp_buffer[filename_len] == ' ')) {

                    if (verbose) {
                        LOG(LOG_INFO, "Copy hash");
                    }

                    memcpy(hash_line.hash1, temp_buffer + filename_len + 1, sizeof(hash_line.hash1));
                    memcpy(hash_line.hash2, temp_buffer + filename_len + 1 + sizeof(hash_line.hash1), sizeof(hash_line.hash2));

                    hash_ok = true;
                }
                else {
                    std::cerr << "File name mismatch: \"" << file_path << "\"" << std::endl << std::endl;
                }
            }
            else {
                if (verbose) {
                    LOG(LOG_INFO, "Hash data v2 or plus");
                }

                struct ReaderBuf
                {
                    char    * remaining_data_buf;
                    ssize_t   remaining_data_length;

                    ssize_t operator()(char * buf, size_t len) {
                        ssize_t number_of_bytes_to_read = std::min<ssize_t>(remaining_data_length, len);
                        if (number_of_bytes_to_read == 0) {
                            return -1;
                        }

                        memcpy(buf, remaining_data_buf, number_of_bytes_to_read);

                        this->remaining_data_buf    += number_of_bytes_to_read;
                        this->remaining_data_length -= number_of_bytes_to_read;

                        return number_of_bytes_to_read;
                    }
                };

                detail::ReaderLine<ReaderBuf> reader({temp_buffer, number_of_bytes_read});
                auto hash_header = detail::read_hash_headers(reader);

                if (detail::read_hash_file_v2(reader, hash_header, infile_is_checksumed, hash_line) != ERR_TRANSPORT_NO_MORE_DATA) {
                    if (!memcmp(hash_line.filename, input_filename.c_str(), filename_len)) {
                        hash_ok = true;
                    }
                    else {
                        std::cerr << "File name mismatch: \"" << file_path << "\"" << std::endl << std::endl;
                    }
                }
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

    const bool is_status_enabled = (infile_version > 1);
    if (!check_mwrm_file<T>(fullfilename, is_status_enabled, hash_line,
            (quick_check ? QUICK_CHECK_LENGTH : 0), args ...)) {
        std::cerr << "File \"" << fullfilename << "\" is invalid!" << std::endl << std::endl;

        exit(-1);
    }

    std::cout << "No error detected during the data verification." << std::endl << std::endl;

    return 0;
}

int app_verifier(int argc, char ** argv, const char * copyright_notice, CryptoContext & cctx)
{
    openlog("verifier", LOG_CONS | LOG_PERROR, LOG_USER);

    Inifile ini;
    ConfigurationLoader cfg_loader(ini.configuration_holder(), CFG_PATH "/" RDPPROXY_INI);

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

    if (infile_is_encrypted) {
        OpenSSL_add_all_digests();

       return check_encrypted_or_checksumed_file<transbuf::icrypto_filename_buf>(
            input_filename, hash_path, fullfilename, quick_check, verbose,
            &cctx);
    }
    else {
        return check_encrypted_or_checksumed_file<transbuf::ifile_buf>(
            input_filename, hash_path, fullfilename, quick_check, verbose);
    }
}
