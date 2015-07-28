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
#include "ssl_calls.hpp"
#include "config.hpp"
//#include "crypto_impl.hpp"
#include "ccryptofile.h"
#include "program_options.hpp"
#include "fdbuf.hpp"

#ifndef HASH_LEN
#define HASH_LEN 64
#endif

static inline bool check_file_hash_sha256( const char * file_path
                    , const Stream & crypto_key
                    , uint8_t * hash_buf
                    , size_t    hash_len
                    , size_t len_to_check) {

    if (SHA256_DIGEST_LENGTH != hash_len){
        return false;
    }

    SslHMAC_Sha256 hmac(crypto_key.get_data(), crypto_key.size());

    io::posix::fdbuf file;
    file.open(file_path, O_RDONLY);
    if (!file.is_open()) {
        TODO("We should log if the file whose signature we check does not exists");
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
        return false;
    }

    file.close();
    uint8_t         hash[SHA256_DIGEST_LENGTH];
    hmac.final(&hash[0], SHA256_DIGEST_LENGTH);

    return (memcmp(hash, hash_buf, hash_len) == 0);
}

static inline int extract_file_info( const char * space_separated_values
                     , Stream & file_name
                     , Stream & _4kb_hash
                     , Stream & full_hash) {
    file_name.reset();
    _4kb_hash.reset();
    full_hash.reset();

    if ((_4kb_hash.get_capacity() < HASH_LEN / 2) || (full_hash.get_capacity() < HASH_LEN / 2)) {
        return -1;
    }

    size_t len = strlen(space_separated_values);

    if (len == 0) {
        return 0;
    }

    const char   * point_end;
    const char   * point_start;
    int            i, c;
    unsigned int   code;
    const char   * psz;

    point_end = (space_separated_values[len - 1] == '\n') ?
                &space_separated_values[len - 1] : &space_separated_values[len];
    if (point_end == space_separated_values) {
        return 0;
    }

    // full hash
    for (point_start = point_end - 1;
         (*point_start != ' ') && (point_start > space_separated_values);
         point_start--);
    if ((*point_start != ' ') || (point_end - (point_start + 1) != HASH_LEN)) {
        return -1;
    }
//LOG(LOG_INFO, "full_hash='%.*s'", (unsigned)(point_end - (point_start + 1)), point_start + 1);
    for (i = 0, c = HASH_LEN / 2, psz = point_start + 1; i < c; i++, psz += 2) {
        sscanf(psz, "%02x", &code);

        full_hash.out_uint8((uint8_t)code);
    }
    full_hash.mark_end();

    // first 4 kb hash
    point_end = point_start;
    for (point_start--;
         (*point_start != ' ') && (point_start > space_separated_values);
         point_start--);
    if ((*point_start != ' ') || (point_end - (point_start + 1) != HASH_LEN)) {
        return -1;
    }
//LOG(LOG_INFO, "_4kb_hash='%.*s'", (unsigned)(point_end - (point_start + 1)), point_start + 1);
    for (i = 0, c = HASH_LEN / 2, psz = point_start + 1; i < c; i++, psz += 2) {
        sscanf(psz, "%02x", &code);

        _4kb_hash.out_uint8((uint8_t)code);
    }
    _4kb_hash.mark_end();

    // end timestamp
    for (/*point_end = point_start, */point_start--;
         (*point_start != ' ') && (point_start > space_separated_values);
         point_start--);
    if (*point_start != ' ') {
        return -1;
    }
//LOG(LOG_INFO, "end timestamp='%.*s'", (unsigned)(point_end - (point_start + 1)), point_start + 1);

    // start timestamp
    for (point_end = point_start, point_start--;
         (*point_start != ' ') && (point_start > space_separated_values);
         point_start--);
    if (*point_start != ' ') {
        return -1;
    }
//LOG(LOG_INFO, "start timestamp='%.*s'", (unsigned)(point_end - (point_start + 1)), point_start + 1);

    // filename
    point_end   = point_start;
    point_start = space_separated_values;
    if ((point_end <= point_start) || (file_name.get_capacity() < (size_t)(point_end - point_start))) {
        return -1;
    }
//LOG(LOG_INFO, "filename='%.*s'", (unsigned)(point_end - point_start), point_start);
    file_name.out_copy_bytes(point_start, (point_end - point_start));
    file_name.out_uint8(0);
    file_name.mark_end();

    return 1;
}

// opaque_stream should be initialized before the first call to read_line.
// opaque_data should be initialize with 1 before the first call to read_line.
template <typename FileDescriptor>
int read_line(  FileDescriptor fd
              , int (*fp_read)(FileDescriptor, char *, unsigned int)
              , Stream & opaque_stream
              , int & opaque_data
              , char * line_buf
              , size_t line_len) {
    char    * internal_line_buf = line_buf;
    size_t    internal_line_len = line_len;
    uint8_t * psz                         ;

    if (line_len == 0) {
        return -1;
    }

    *line_buf = '\0';

    do {
        // Finds newline in buffer.
        for (psz = opaque_stream.get_data();
             (psz != opaque_stream.p) && ((psz - opaque_stream.get_data()) < (ssize_t)(internal_line_len - 1));
             psz++) {
            if (*psz == '\n') {
                psz++;

                break;
            }
        }

        if (psz > opaque_stream.get_data()) {
            size_t len = psz - opaque_stream.get_data();

            memcpy(internal_line_buf, opaque_stream.get_data(), len);

            internal_line_buf[len] = '\0';

            memmove(opaque_stream.get_data(), psz, opaque_stream.p - opaque_stream.get_data() - len);

            opaque_stream.p -= len;

            // A newline is read or ...
            if ((internal_line_buf[len - 1] == '\n') ||
                // ... buffer is full.
                (len == internal_line_len - 1)) {
                return len;
            }

            internal_line_buf += len;
            internal_line_len -= len;
        }

        if (opaque_data <= 0) {
            if (internal_line_len != line_len) {
                return line_len - internal_line_len;
            }

            return opaque_data;
        }

        int number_of_bytes_read = fp_read(fd, reinterpret_cast<char *>(opaque_stream.p), opaque_stream.tailroom());

        if (number_of_bytes_read > 0) {
            opaque_stream.p += number_of_bytes_read;
        }
        else {
            opaque_data = number_of_bytes_read;
        }
    }
    while (true);
}

bool check_file_hash(const char * file_path, const Stream & crypto_key, const char hash[HASH_LEN], bool quick_check)
{
    StaticStream _4kb_hash(hash, HASH_LEN / 2);
    StaticStream full_hash(hash + (HASH_LEN / 2), HASH_LEN / 2);

    if (quick_check){
        return check_file_hash_sha256(file_path, /*ss_hmac_key*/crypto_key,
                                      _4kb_hash.get_data(), _4kb_hash.size(), 4096);
    }
    return check_file_hash_sha256(file_path, /*ss_hmac_key*/crypto_key,
                                      full_hash.get_data(), full_hash.size(), 0);
}

// opaque_stream should be initialized before the first call to read_line.
// opaque_data should be initialize with 1 before the first call to read_line.
/*
static inline int crypto_read_line(crypto_file * cf_struct
              , Stream & opaque_stream
              , int & opaque_data
              , char * line_buf
              , size_t line_len) {
    char    * internal_line_buf = line_buf;
    size_t    internal_line_len = line_len;
    uint8_t * psz                         ;
    size_t    len                         ;
    int       number_of_bytes_read        ;

    if (line_len == 0) {
        return -1;
    }

    *line_buf = '\0';

    do {
        // Finds newline in buffer.
        for (psz = opaque_stream.get_data();
             (psz != opaque_stream.p) && ((psz - opaque_stream.get_data()) < (ssize_t)(internal_line_len - 1));
             psz++) {
            if (*psz == '\n') {
                psz++;

                break;
            }
        }

        if (psz > opaque_stream.get_data()) {
            len = psz - opaque_stream.get_data();

            memcpy(internal_line_buf, opaque_stream.get_data(), len);

            internal_line_buf[len] = '\0';

            memmove(opaque_stream.get_data(), psz, opaque_stream.p - opaque_stream.get_data() - len);

            opaque_stream.p -= len;

            // A newline is read or ...
            if ((internal_line_buf[len - 1] == '\n') ||
                // ... buffer is full.
                (len == internal_line_len - 1)) {
                return len;
            }

            internal_line_buf += len;
            internal_line_len -= len;
        }

        if (opaque_data <= 0) {
            if (internal_line_len != line_len) {
                return line_len - internal_line_len;
            }

            return opaque_data;
        }

        number_of_bytes_read = crypto_read(cf_struct, reinterpret_cast<char *>(opaque_stream.p), opaque_stream.tailroom());

        if (number_of_bytes_read > 0) {
            opaque_stream.p += number_of_bytes_read;
        }
        else {
            opaque_data = number_of_bytes_read;
        }
    }
    while (true);
}
*/

bool check_mwrm_file(CryptoContext * cctx, const char * file_path, const char hash[HASH_LEN], bool quick_check) {
    TODO("Add unit test for this function")
    bool result = false;

    StaticStream ss_hmac_key(cctx->hmac_key, sizeof(cctx->hmac_key));

    if (check_file_hash(file_path, ss_hmac_key, hash, false) == true) {

        unsigned char derivator[DERIVATOR_LENGTH];
        get_derivator(file_path, derivator, DERIVATOR_LENGTH);
        unsigned char trace_key[CRYPTO_KEY_LENGTH]; // derived key for cipher
        if (compute_hmac(trace_key, cctx->crypto_key, derivator) == -1){
            return false;
        }

        int system_fd = open(file_path, O_RDONLY, 0600);
        if (system_fd == -1){
            LOG(LOG_ERR, "failed opening=%s\n", file_path);
            return false;
        }

        crypto_file * cf_struct = crypto_open_read(system_fd, trace_key, cctx);
        if (!cf_struct){
            close(system_fd);
        }

        if (cf_struct != nullptr) {
            BStream opaque_stream(2048);

            int opaque_data = 1;

            char header0[1024];
            char header1[1024];
            char header2[1024];

            if (  (read_line(cf_struct, crypto_read, opaque_stream, opaque_data, header0, sizeof(header0)) > 0)
               && (read_line(cf_struct, crypto_read, opaque_stream, opaque_data, header1, sizeof(header1)) > 0)
               && (read_line(cf_struct, crypto_read, opaque_stream, opaque_data, header2, sizeof(header2)) > 0)
               ) {
                result = true;

                BStream file_name(1024);
                BStream _4kb_hash(HASH_LEN);
                BStream full_hash(HASH_LEN);

                char line[1024];
                //int  line_len;

                while ((/*line_len = */read_line(cf_struct, crypto_read, opaque_stream, opaque_data, line, sizeof(line))) > 0) {
                    int extract_file_info_result = extract_file_info(line, file_name, _4kb_hash, full_hash);

                    if ((extract_file_info_result > 0)
                        && (((quick_check)
                            && (check_file_hash_sha256(reinterpret_cast<const char *>(file_name.get_data())
                                        , ss_hmac_key
                                        , _4kb_hash.get_data()
                                        , _4kb_hash.size()
                                        , 4096) == false)
                           )
                          || ((!quick_check)
                            && (check_file_hash_sha256(reinterpret_cast<const char *>(file_name.get_data())
                                        , ss_hmac_key
                                        , full_hash.get_data()
                                        , full_hash.size()
                                        , 0) == false)
                           ))) {
                        result = false; break;
                    }
                    else if (extract_file_info_result < 0) {
                        result = false; break;
                    }
                }
            }
        }

        unsigned char ignore[HASH_LEN];

        crypto_close(cf_struct, ignore, cctx->hmac_key);
    }

    return result;
}

void make_file_path(const char * directory_name, const char * file_name,  char * file_path_buf, size_t file_path_len) {
    snprintf(file_path_buf, file_path_len, "%s%s%s", directory_name,
        ((directory_name[strlen(directory_name) - 1] == '/') ? "" : "/"),
        file_name);

    file_path_buf[file_path_len - 1] = '\0';
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

    if (!infile_is_encrypted) {
        std::cout << "Input file is unencrypted.\n";
        return 0;
    }

    CryptoContext cctx;
    memset(&cctx, 0, sizeof(cctx));
    if (int status = crypto_context_initializer(cctx)) {
        return status;
    }

    OpenSSL_add_all_digests();

    /*****************
    * Load file hash *
    *****************/
    char hash[HASH_LEN];

    {
        char file_path[1024] = {};
//        char temp_path[1024] = {};
//        char temp_basename[1024] = {};
//        char temp_extension[256] = {};
//        char temp_filename[2048] = {};

//        canonical_path(input_filename.c_str(), temp_path, sizeof(temp_path), temp_basename, sizeof(temp_basename), temp_extension, sizeof(temp_extension));
        //snprintf(temp_filename, sizeof(temp_filename), "%s%s", temp_basename, temp_extension);
//        temp_filename[sizeof(temp_filename) - 1] = '\0';

//        std::cout << "temp_filename: \"" << temp_filename << "\"" << std::endl << std::endl;
//        size_t filename_len = strlen(temp_filename);
        size_t filename_len = input_filename.length();

        bool hash_ok = false;

        make_file_path(hash_path.c_str(), input_filename.c_str(), file_path, sizeof(file_path));
        std::cout << "hash file path: \"" << file_path << "\"." << std::endl;

        char   temp_buffer[4096];
        char * buf;

        try {
            CryptoInFilenameTransport in_hash_t(&cctx, file_path);
            if (verbose) {
                LOG(LOG_INFO, "Transport created");
            }

            buf = temp_buffer;

            memset(temp_buffer, 0, sizeof(temp_buffer));

            const size_t hash_data_length = filename_len + 1 + HASH_LEN;
            if (verbose) {
                LOG(LOG_INFO, "Hash data length=", hash_data_length);
            }
            in_hash_t.recv(&buf, hash_data_length);

            if (verbose) {
                LOG(LOG_INFO, "Hash data received");
            }

            // Filename HASH_64_BYTES
            //         ^
            //         |
            //     separator
//            if (!memcmp(temp_buffer, temp_filename, filename_len) &&
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

    /*****************
    * Check mwrm file *
    *****************/
    if (check_mwrm_file(&cctx, fullfilename, hash, quick_check) == false) {
        std::cerr << "File \"" << fullfilename << "\" is invalid!" << std::endl << std::endl;

        exit(-1);
    }

    std::cout << "No error detected during the data verification." << std::endl << std::endl;

    return 0;
}
