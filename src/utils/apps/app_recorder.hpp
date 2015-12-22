/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2014
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_UTILS_APPS_APP_RECORDER_HPP
#define REDEMPTION_UTILS_APPS_APP_RECORDER_HPP

#include <signal.h>

#include "FileToChunk.hpp"
#include "ChunkToFile.hpp"
#include "out_meta_sequence_transport.hpp"
#include "crypto_out_meta_sequence_transport.hpp"
#include "in_meta_sequence_transport.hpp"
#include "recording_progress.hpp"
#include "iter.hpp"
#include "crypto_in_meta_sequence_transport.hpp"
#include "program_options/program_options.hpp"

#include <iostream>
#include <vector>
#include <string>

template<class CaptureMaker, class... ExtraArguments>
int recompress_or_record( std::string const & input_filename, std::string & output_filename
                        , int capture_bpp, int wrm_compression_algorithm_
                        , Inifile & ini, bool remove_input_file
                        , CryptoContext & cctx, Random & rnd, bool infile_is_encrypted
                        , bool auto_output_file, uint32_t begin_cap, uint32_t end_cap
                        , uint32_t order_count, uint32_t clear, unsigned zoom
                        , unsigned png_width, unsigned png_height
                        , bool show_file_metadata, bool show_statistics
                        , bool force_record, uint32_t verbose
                        , ExtraArguments&&... extra_argument);

template<typename InWrmTrans>
unsigned get_file_count( InWrmTrans & in_wrm_trans, uint32_t & begin_cap, uint32_t & end_cap, timeval & begin_record
                       , timeval & end_record);

template<typename InWrmTrans>
void remove_file(InWrmTrans & in_wrm_trans, const char * hash_path, const char * infile_path
                , const char * infile_basename, const char * infile_extension, bool is_encrypted);

template<class CaptureMaker, class... ExtraArguments>
static int do_record( Transport & in_wrm_trans, const timeval begin_record, const timeval end_record
                    , const timeval begin_capture, const timeval end_capture, std::string const & output_filename
                    , int capture_bpp, int wrm_compression_algorithm_
                    , Inifile & ini, unsigned file_count, uint32_t order_count, uint32_t clear, unsigned zoom
                    , unsigned png_width, unsigned png_height
                    , bool show_file_metadata, bool show_statistics, uint32_t verbose
                    , ExtraArguments && ... extra_argument);

static int do_recompress( CryptoContext & cctx, Transport & in_wrm_trans, const timeval begin_record
                        , int wrm_compression_algorithm_
                        , std::string const & output_filename, Inifile & ini, uint32_t verbose);


static void show_statistics(FileToGraphic::Statistics const & statistics);

static void show_metadata(FileToGraphic const & player);

static void raise_error(std::string const & output_filename, int code, const char * message, uint32_t verbose);

int is_encrypted_file(const char * input_filename, bool & infile_is_encrypted);


static const signed USE_ORIGINAL_COMPRESSION_ALGORITHM = 0xFFFFFFFF;
static const signed USE_ORIGINAL_COLOR_DEPTH           = 0xFFFFFFFF;


bool program_requested_to_shutdown = false;

void shutdown(int sig)
{
    LOG(LOG_INFO, "shutting down : signal %d pid=%d\n", sig, getpid());

    program_requested_to_shutdown = true;
}

void init_signals(void)
{
    struct sigaction sa;

    sa.sa_flags = 0;

    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGTERM);

    sa.sa_handler = shutdown;
    sigaction(SIGTERM, &sa, nullptr);
}


template<
    class CaptureMaker, class AddProgramOtion, class ParseFormat
  , class HasExtraCapture, class... ExtraArguments>
int app_recorder( int argc, char ** argv, const char * copyright_notice
                , AddProgramOtion add_prog_option, ParseFormat parse_format
                , std::string & config_filename, Inifile & ini
                , CryptoContext & cctx, Random & rnd, HasExtraCapture has_extra_capture
                , ExtraArguments&&... extra_argument)
{
    openlog("redrec", LOG_CONS | LOG_PERROR, LOG_USER);

    init_signals();

    unsigned png_width  = 0;
    unsigned png_height = 0;

    std::string input_filename;
    std::string output_filename;

    uint32_t    verbose            = 0;
    uint32_t    clear              = 1; // default on
    uint32_t    begin_cap          = 0;
    uint32_t    end_cap            = 0;
    uint32_t    png_limit          = 10;
    uint32_t    png_interval       = 60;
    uint32_t    wrm_frame_interval = 100;
    uint32_t    wrm_break_interval = 86400;
    uint32_t    order_count        = 0;
    unsigned    zoom               = 100;
    bool        show_file_metadata = false;
    bool        show_statistics    = false;
    bool        auto_output_file   = false;
    bool        remove_input_file  = false;

    std::string wrm_compression_algorithm;  // output compression algorithm.
    std::string wrm_color_depth;
    std::string wrm_encryption;
    std::string png_geometry;

    program_options::options_description desc({
        {'h', "help", "produce help message"},
        {'v', "version", "show software version"},
        {'o', "output-file", &output_filename, "output base filename"},
        {'i', "input-file", &input_filename, "input base filename"},

        {'b', "begin", &begin_cap, "begin capture time (in seconds), default=none"},
        {'e', "end", &end_cap, "end capture time (in seconds), default=none"},
        {"count", &order_count, "Number of orders to execute before stopping, default=0 execute all orders"},

        {'l', "png_limit", &png_limit, "maximum number of png files to create (remove older), default=10, 0 will disable png capture"},
        {'n', "png_interval", &png_interval, "time interval between png captures, default=60 seconds"},

        {'r', "frameinterval", &wrm_frame_interval, "time between consecutive capture frames (in 100/th of seconds), default=100 one frame per second"},

        {'k', "breakinterval", &wrm_break_interval, "number of seconds between splitting wrm files in seconds(default, one wrm every day)"},

        {'p', "png", "enable png capture"},
        {'w', "wrm", "enable wrm capture"},

        {"clear", &clear, "clear old capture files with same prefix (default on)"},
        {"verbose", &verbose, "more logs"},
        {"zoom", &zoom, "scaling factor for png capture (default 100%)"},
        {'g', "png-geometry", & png_geometry, "png capture geometry (Ex. 160x120)"},
        {'m', "meta", "show file metadata"},
        {'s', "statistics", "show statistics"},

        //{"compression,z", &wrm_compression_algorithm, "wrm compression algorithm (default=original, none, gzip, snappy, lzma)"},
        {'z', "compression", &wrm_compression_algorithm, "wrm compression algorithm (default=original, none, gzip, snappy)"},
        {'d', "color-depth", &wrm_color_depth,           "wrm color depth (default=original, 16, 24)"},
        {'y', "encryption",  &wrm_encryption,            "wrm encryption (default=original, enable, disable)"},

        {"auto-output-file",  "append suffix to input base filename to generate output base filename automatically"},
        {"remove-input-file", "remove input file"},

        {"config-file", &config_filename, "used an another ini file"},
    });

    add_prog_option(desc);


    auto options = program_options::parse_command_line(argc, argv, desc);

    if (options.count("help") > 0) {
        std::cout << copyright_notice;
        std::cout << "\n\nUsage: redrec [options]\n\n";
        std::cout << desc << "\n\n";
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

    show_file_metadata = (options.count("meta"             ) > 0);
    show_statistics    = (options.count("statistics"       ) > 0);
    auto_output_file   = (options.count("auto-output-file" ) > 0);
    remove_input_file  = (options.count("remove-input-file") > 0);

    if (!show_file_metadata && !show_statistics && !auto_output_file && output_filename.empty()) {
        std::cerr << "Missing output filename : use -o filename\n\n";
        return -1;
    }

    if (!output_filename.empty() && auto_output_file) {
        std::cerr << "Conflicting options : --output-file and --auto-output-file\n\n";
        return -1;
    }

    if ((options.count("zoom") > 0) & (options.count("png-geometry") > 0)) {
        std::cerr << "Conflicting options : --zoom and --png-geometry\n\n";
        return -1;
    }

    if (options.count("png-geometry") > 0) {
        const char * png_geometry_c = png_geometry.c_str();
        const char * separator      = strchr(png_geometry_c, 'x');
        int          png_w          = atoi(png_geometry_c);
        int          png_h          = 0;
        if (separator) {
            png_h = atoi(separator + 1);
        }
        if (!png_w || !png_h) {
            std::cerr << "Invalide png geometry\n\n";
            return -1;
        }
        png_width  = png_w;
        png_height = png_h;
        std::cout << "png-geometry: " << png_width << "x" << png_height << std::endl;
    }

    { ConfigurationLoader cfg_loader_full(ini, config_filename.c_str()); }

    int wrm_compression_algorithm_;

    if (options.count("compression") > 0) {
             if (wrm_compression_algorithm == "none") {
//            ini.set<cfg::video::wrm_compression_algorithm>(0);
            wrm_compression_algorithm_ = 0;
        }
        else if (wrm_compression_algorithm == "gzip") {
//            ini.set<cfg::video::wrm_compression_algorithm>(1);
            wrm_compression_algorithm_ = 1;
        }
        else if (wrm_compression_algorithm == "snappy") {
//            ini.set<cfg::video::wrm_compression_algorithm>(2);
            wrm_compression_algorithm_ = 2;
        }
        else if (wrm_compression_algorithm == "original") {
//            ini.set<cfg::video::wrm_compression_algorithm>(USE_ORIGINAL_COMPRESSION_ALGORITHM);
            wrm_compression_algorithm_ = USE_ORIGINAL_COMPRESSION_ALGORITHM;
        }
        else {
            std::cerr << "Unknown wrm compression algorithm\n\n";
            return -1;
        }
    }
    else {
//        ini.set<cfg::video::wrm_compression_algorithm>(USE_ORIGINAL_COMPRESSION_ALGORITHM);
        wrm_compression_algorithm_ = USE_ORIGINAL_COMPRESSION_ALGORITHM;
    }

    int capture_bpp = 16;

    if (options.count("color-depth") > 0) {
             if (wrm_color_depth == "16") {
//            ini.set<cfg::video::wrm_color_depth_selection_strategy>(16);
            capture_bpp = 16;
        }
        else if (wrm_color_depth == "24") {
//            ini.set<cfg::video::wrm_color_depth_selection_strategy>(24);
            capture_bpp = 24;
        }
        else if (wrm_color_depth == "original") {
//            ini.set<cfg::video::wrm_color_depth_selection_strategy>(USE_ORIGINAL_COLOR_DEPTH);
            capture_bpp = USE_ORIGINAL_COLOR_DEPTH;
        }
        else {
            std::cerr << "Unknown wrm color depth\n\n";
            return -1;
        }
    }
    else {
//        ini.set<cfg::video::wrm_color_depth_selection_strategy>(USE_ORIGINAL_COLOR_DEPTH);
        capture_bpp = USE_ORIGINAL_COLOR_DEPTH;
    }

    ini.set<cfg::video::png_limit>(png_limit);
    ini.set<cfg::video::png_interval>(png_interval);
    ini.set<cfg::video::frame_interval>(wrm_frame_interval);
    ini.set<cfg::video::break_interval>(wrm_break_interval);
    ini.get_ref<cfg::video::capture_flags>() &= ~(configs::CaptureFlags::wrm | configs::CaptureFlags::png);
    if (options.count("wrm") > 0) {
        ini.get_ref<cfg::video::capture_flags>() |= configs::CaptureFlags::wrm;
    }
    if (options.count("png") > 0) {
        ini.get_ref<cfg::video::capture_flags>() |= configs::CaptureFlags::png;
    }

    if (int status = parse_format(ini, options, output_filename)) {
        return status;
    }

    ini.set<cfg::video::rt_display>(bool(ini.get<cfg::video::capture_flags>() & configs::CaptureFlags::png));

/*
    {
        char temp_path[1024]     = {};
        char temp_basename[1024] = {};
        char temp_extension[256] = {};

        canonical_path(input_filename.c_str(), temp_path, sizeof(temp_path), temp_basename, sizeof(temp_basename), temp_extension, sizeof(temp_extension), verbose);

        if (!temp_path[0]) {
            input_filename  = ini.get<cfg::video::record_path>();
            const size_t path_length = input_filename.length();
            if (path_length && (input_filename[path_length - 1] != '/')) {
                input_filename += '/';
            }
            input_filename += temp_basename;
            input_filename += temp_extension;
        }
    }
*/
    [&input_filename] (const char * record_path) {
        std::string directory          ;
        std::string filename           ;
        std::string extension = ".mwrm";

        ParsePath(input_filename.c_str(), directory, filename, extension);
        if (!directory.size()) {
            if (file_exist(input_filename.c_str())) {
                directory = "./";
            }
            else {
                directory = record_path;
            }
        }
        MakePath(input_filename, directory.c_str(), filename.c_str(), extension.c_str());
    } (ini.get<cfg::video::record_path>().c_str());
    std::cout << "Input file is \"" << input_filename << "\".\n";

    bool infile_is_encrypted;
    if (is_encrypted_file(input_filename.c_str(), infile_is_encrypted) == -1) {
        std::cerr << "Input file is absent.\n";
        return -1;
    }

    if (options.count("encryption") > 0) {
             if (0 == strcmp(wrm_encryption.c_str(), "enable")) {
            ini.set<cfg::globals::trace_type>(configs::TraceType::cryptofile);
        }
        else if (0 == strcmp(wrm_encryption.c_str(), "disable")) {
            ini.set<cfg::globals::trace_type>(configs::TraceType::localfile);
        }
        else if (0 == strcmp(wrm_encryption.c_str(), "original")) {
            ini.set<cfg::globals::trace_type>(infile_is_encrypted ? configs::TraceType::cryptofile : configs::TraceType::localfile);
        }
        else {
            std::cerr << "Unknown wrm encryption parameter\n\n";
            return -1;
        }
    }
    else {
        ini.set<cfg::globals::trace_type>(infile_is_encrypted ? configs::TraceType::cryptofile : configs::TraceType::localfile);
    }

    if (infile_is_encrypted || (ini.get<cfg::globals::trace_type>() == configs::TraceType::cryptofile)) {
        OpenSSL_add_all_digests();
    }

    return recompress_or_record<CaptureMaker>(
        input_filename, output_filename, capture_bpp, wrm_compression_algorithm_, ini
      , remove_input_file
      , cctx, rnd, infile_is_encrypted, auto_output_file
      , begin_cap, end_cap, order_count, clear, zoom
      , png_width, png_height
      , show_file_metadata, show_statistics
      , has_extra_capture(ini)
      , verbose
      , std::forward<ExtraArguments>(extra_argument)...);
}


inline
int is_encrypted_file(const char * input_filename, bool & infile_is_encrypted)
{
    infile_is_encrypted = false;
    const int fd_test = open(input_filename, O_RDONLY);
    if (fd_test != -1) {
        uint32_t magic_test;
        TODO("Not portable code endianess, use byte array instead")
        ssize_t res_test = read(fd_test, &magic_test, sizeof(magic_test));
        if ((res_test == sizeof(magic_test)) &&
            (magic_test == WABCRYPTOFILE_MAGIC)) {
            infile_is_encrypted = true;
            std::cout << "Input file is encrypted.\n";
        }
        close(fd_test);

        return 0;
    }

    return -1;
}


template<class CaptureMaker, class... ExtraArguments>
int recompress_or_record( std::string const & input_filename, std::string & output_filename
                        , int capture_bpp, int wrm_compression_algorithm_
                        , Inifile & ini, bool remove_input_file
                        , CryptoContext & cctx, Random & rnd, bool infile_is_encrypted
                        , bool auto_output_file, uint32_t begin_cap, uint32_t end_cap
                        , uint32_t order_count, uint32_t clear, unsigned zoom
                        , unsigned png_width, unsigned png_height
                        , bool show_file_metadata, bool show_statistics
                        , bool force_record, uint32_t verbose
                        , ExtraArguments&&... extra_argument)
{
/*
    char infile_path     [1024] = "./"          ;   // default value, actual one should come from output_filename
    char infile_basename [1024] = "redrec_input";   // default value, actual one should come from output_filename
    char infile_extension[ 128] = ".mwrm"       ;

    canonical_path( input_filename.c_str()
                  , infile_path
                  , sizeof(infile_path)
                  , infile_basename
                  , sizeof(infile_basename)
                  , infile_extension
                  , sizeof(infile_extension)
                  , verbose
                  );
    if (verbose) {
        std::cout << "\nInput file path: " << infile_path << infile_basename << infile_extension << std::endl;
    }
*/
    std::string infile_path;
    std::string infile_basename;
    std::string infile_extension;
    ParsePath(input_filename.c_str(), infile_path, infile_basename, infile_extension);

    char infile_prefix[4096];
    snprintf(infile_prefix, sizeof(infile_prefix), "%s%s", infile_path.c_str(), infile_basename.c_str());

    if (auto_output_file) {
        output_filename =  infile_path;
        output_filename += infile_basename;
        output_filename += "-redrec";
        output_filename += infile_extension;

        std::cout << "\nOutput file is \"" << output_filename << "\" (autogenerated)." << std::endl;
    }
    else if (output_filename.size()) {
        [&output_filename] () {
            std::string directory = PNG_PATH "/"; // default value, actual one should come from output_filename
            std::string filename                ;
            std::string extension = ".mwrm"     ;

            ParsePath(output_filename.c_str(), directory, filename, extension);
            MakePath(output_filename, directory.c_str(), filename.c_str(), extension.c_str());
        } ();
        std::cout << "Output file is \"" << output_filename << "\".\n";
    }

    TODO("before continuing to work with input file, check if it's mwrm or wrm and use right object in both cases")

    TODO("also check if it contains any wrm at all and at wich one we should start depending on input time")
    TODO("if start and stop time are outside wrm, users should also be warned")

    timeval  begin_record = { 0, 0 };
    timeval  end_record   = { 0, 0 };
    unsigned file_count   = 0;
    try {
        if (infile_is_encrypted == false) {
            InMetaSequenceTransport in_wrm_trans_tmp(infile_prefix, infile_extension.c_str());
            file_count = get_file_count(in_wrm_trans_tmp, begin_cap, end_cap, begin_record, end_record);
        }
        else {
            CryptoInMetaSequenceTransport in_wrm_trans_tmp(&cctx, infile_prefix, infile_extension.c_str());
            file_count = get_file_count(in_wrm_trans_tmp, begin_cap, end_cap, begin_record, end_record);
        }
    }
    catch (const Error & e) {
        if (e.id == static_cast<unsigned>(ERR_TRANSPORT_NO_MORE_DATA)) {
            std::cerr << "Asked time not found in mwrm file\n";
        }
        else {
            std::cerr << "Exception code: " << e.id << std::endl;
        }
        const bool msg_with_error_id = false;
        raise_error(output_filename, e.id, e.errmsg(msg_with_error_id), verbose);
        return -1;
    };

    auto run = [&](Transport && trans, ExtraArguments&&... extra_argument) {
        timeval begin_capture = {0, 0};
        timeval end_capture = {0, 0};

        int result = -1;
        try {
            result = (
                force_record
             || bool(ini.get<cfg::video::capture_flags>() & configs::CaptureFlags::png)
//             || ini.get<cfg::video::wrm_color_depth_selection_strategy>() != USE_ORIGINAL_COLOR_DEPTH
             || capture_bpp != USE_ORIGINAL_COLOR_DEPTH
             || show_file_metadata
             || show_statistics
             || file_count > 1
             || order_count)
                ? ((verbose ? void(std::cout << "[A]"<< std::endl) : void())
                  , do_record<CaptureMaker>(
                      trans, begin_record, end_record, begin_capture, end_capture
                    , output_filename, capture_bpp, wrm_compression_algorithm_, ini, rnd
                    , file_count, order_count, clear, zoom
                    , png_width, png_height
                    , show_file_metadata, show_statistics, verbose
                    , std::forward<ExtraArguments>(extra_argument)...
                    )
                )
                : ((verbose ? void(std::cout << "[B]"<< std::endl) : void())
                  , do_recompress(cctx, trans, begin_record, wrm_compression_algorithm_, output_filename, ini, verbose)
                )
            ;
        }
        catch (const Error & e) {
            const bool msg_with_error_id = false;
            raise_error(output_filename, e.id, e.errmsg(msg_with_error_id), verbose);
        }

        if (!result && remove_input_file) {
            if (infile_is_encrypted == false) {
                InMetaSequenceTransport in_wrm_trans_tmp(infile_prefix, infile_extension.c_str());
                remove_file( in_wrm_trans_tmp, ini.get<cfg::video::hash_path>(), infile_path.c_str()
                           , infile_basename.c_str(), infile_extension.c_str()
                           , infile_is_encrypted);
            }
            else {
                CryptoInMetaSequenceTransport in_wrm_trans_tmp(&cctx, infile_prefix, infile_extension.c_str());
                remove_file( in_wrm_trans_tmp, ini.get<cfg::video::hash_path>(), infile_path.c_str()
                           , infile_basename.c_str(), infile_extension.c_str()
                           , infile_is_encrypted);
            }
        }

        std::cout << std::endl;

        return result;
    };

    return infile_is_encrypted
        ? run( CryptoInMetaSequenceTransport(&cctx, infile_prefix, infile_extension.c_str())
             , std::forward<ExtraArguments>(extra_argument)...)
        : run( InMetaSequenceTransport(infile_prefix, infile_extension.c_str())
             , std::forward<ExtraArguments>(extra_argument)...);
}

template<typename InWrmTrans>
unsigned get_file_count( InWrmTrans & in_wrm_trans, uint32_t & begin_cap, uint32_t & end_cap, timeval & begin_record
                       , timeval & end_record) {
    in_wrm_trans.next();
    begin_record.tv_sec = in_wrm_trans.begin_chunk_time();
    TODO("a negative time should be a time relative to end of movie")
    REDOC("less than 1 year means we are given a time relatve to beginning of movie")
    if (begin_cap && (begin_cap < 31536000)) {  // less than 1 year, it is relative not absolute timestamp
        // begin_capture.tv_usec is 0
        begin_cap += in_wrm_trans.begin_chunk_time();
    }
    if (end_cap && (end_cap < 31536000)) { // less than 1 year, it is relative not absolute timestamp
        // begin_capture.tv_usec is 0
        end_cap += in_wrm_trans.begin_chunk_time();
    }
    while (begin_cap >= in_wrm_trans.end_chunk_time()) {
        in_wrm_trans.next();
    }
    unsigned result = in_wrm_trans.get_seqno();
    try {
        do {
            end_record.tv_sec = in_wrm_trans.end_chunk_time();
            in_wrm_trans.next();
        }
        while (true);
    }
    catch (const Error & e) {
        if (e.id != static_cast<unsigned>(ERR_TRANSPORT_NO_MORE_DATA)) {
            throw;
        }
    };
    return result;
}


template<typename InWrmTrans>
void remove_file( InWrmTrans & in_wrm_trans, const char * hash_path, const char * infile_path
                , const char * infile_basename, const char * infile_extension, bool is_encrypted) {
    std::vector<std::string> files;

    char infile_fullpath[2048];
    if (is_encrypted) {
        snprintf(infile_fullpath, sizeof(infile_fullpath), "%s%s%s", hash_path, infile_basename, infile_extension);
        files.push_back(infile_fullpath);
    }
    snprintf(infile_fullpath, sizeof(infile_fullpath), "%s%s%s", infile_path, infile_basename, infile_extension);
    files.push_back(infile_fullpath);

    try {
        do {
            in_wrm_trans.next();
            files.push_back(in_wrm_trans.path());
        }
        while (true);
    }
    catch (const Error & e) {
        if (e.id != ERR_TRANSPORT_NO_MORE_DATA) {
            throw;
        }
    };

    std::cout << std::endl;
    for (auto & s : iter(files.rbegin(), files.rend())) {
        unlink(s.c_str());
        std::cout << "Removed : " << s << std::endl;
    }
}

inline
static int do_recompress( CryptoContext & cctx, Transport & in_wrm_trans, const timeval begin_record
                        , int wrm_compression_algorithm_
                        , std::string const & output_filename, Inifile & ini, uint32_t verbose) {
    FileToChunk player(&in_wrm_trans, 0);

/*
    char outfile_path     [1024] = PNG_PATH "/"   ; // default value, actual one should come from output_filename
    char outfile_basename [1024] = "redrec_output"; // default value, actual one should come from output_filename
    char outfile_extension[1024] = ""             ; // extension is ignored for targets anyway

    canonical_path( output_filename.c_str()
                  , outfile_path
                  , sizeof(outfile_path)
                  , outfile_basename
                  , sizeof(outfile_basename)
                  , outfile_extension
                  , sizeof(outfile_extension)
                  , verbose
                  );
*/
    std::string outfile_path;
    std::string outfile_basename;
    std::string outfile_extension;
    ParsePath(output_filename.c_str(), outfile_path, outfile_basename, outfile_extension);

    if (verbose) {
        std::cout << "Output file path: " << outfile_path << outfile_basename << outfile_extension << '\n' << std::endl;
    }

    if (recursive_create_directory(outfile_path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP, ini.get<cfg::video::capture_groupid>()) != 0) {
        std::cerr << "Failed to create directory: \"" << outfile_path << "\"" << std::endl;
    }

//    if (ini.get<cfg::video::wrm_compression_algorithm>() == USE_ORIGINAL_COMPRESSION_ALGORITHM) {
//        ini.set<cfg::video::wrm_compression_algorithm>(player.info_compression_algorithm);
//    }
    ini.set<cfg::video::wrm_compression_algorithm>(
        ((wrm_compression_algorithm_ == USE_ORIGINAL_COMPRESSION_ALGORITHM) ?
         player.info_compression_algorithm :
         wrm_compression_algorithm_));

    int return_code = 0;
    try {
        auto run = [&](Transport && trans) {
            {
                ChunkToFile recorder( &trans

                                    , player.info_width
                                    , player.info_height
                                    , player.info_bpp
                                    , player.info_cache_0_entries
                                    , player.info_cache_0_size
                                    , player.info_cache_1_entries
                                    , player.info_cache_1_size
                                    , player.info_cache_2_entries
                                    , player.info_cache_2_size

                                    , player.info_number_of_cache
                                    , player.info_use_waiting_list

                                    , player.info_cache_0_persistent
                                    , player.info_cache_1_persistent
                                    , player.info_cache_2_persistent

                                    , player.info_cache_3_entries
                                    , player.info_cache_3_size
                                    , player.info_cache_3_persistent
                                    , player.info_cache_4_entries
                                    , player.info_cache_4_size
                                    , player.info_cache_4_persistent

                                    , ini);

                player.add_consumer(&recorder);

                player.play(program_requested_to_shutdown);
            }

            if (program_requested_to_shutdown) {
                trans.request_full_cleaning();
            }
        };

        if (ini.get<cfg::globals::trace_type>() == configs::TraceType::cryptofile) {
            run(CryptoOutMetaSequenceTransport(
                &cctx,
                outfile_path.c_str(), ini.get<cfg::video::hash_path>(), outfile_basename.c_str(),
                begin_record, player.info_width, player.info_height,
                ini.get<cfg::video::capture_groupid>()
            ));
        }
        else {
            run(OutMetaSequenceTransport(
                outfile_path.c_str(), ini.get<cfg::video::hash_path>(), outfile_basename.c_str(),
                begin_record, player.info_width, player.info_height,
                ini.get<cfg::video::capture_groupid>()
            ));
        }
    }
    catch (...) {
        return_code = -1;
    }

    return return_code;
}   // do_recompress

inline
static void show_statistics(FileToGraphic::Statistics const & statistics) {
    std::cout
    << "\nDstBlt                : " << statistics.DstBlt
    << "\nMultiDstBlt           : " << statistics.MultiDstBlt
    << "\nPatBlt                : " << statistics.PatBlt
    << "\nMultiPatBlt           : " << statistics.MultiPatBlt
    << "\nOpaqueRect            : " << statistics.OpaqueRect
    << "\nMultiOpaqueRect       : " << statistics.MultiOpaqueRect
    << "\nScrBlt                : " << statistics.ScrBlt
    << "\nMultiScrBlt           : " << statistics.MultiScrBlt
    << "\nMemBlt                : " << statistics.MemBlt
    << "\nMem3Blt               : " << statistics.Mem3Blt
    << "\nLineTo                : " << statistics.LineTo
    << "\nGlyphIndex            : " << statistics.GlyphIndex
    << "\nPolyline              : " << statistics.Polyline

    << "\nCacheBitmap           : " << statistics.CacheBitmap
    << "\nCacheColorTable       : " << statistics.CacheColorTable
    << "\nCacheGlyph            : " << statistics.CacheGlyph

    << "\nFrameMarker           : " << statistics.FrameMarker

    << "\nBitmapUpdate          : " << statistics.BitmapUpdate

    << "\nCachePointer          : " << statistics.CachePointer
    << "\nPointerIndex          : " << statistics.PointerIndex

    << "\ngraphics_update_chunk : " << statistics.graphics_update_chunk
    << "\nbitmap_update_chunk   : " << statistics.bitmap_update_chunk
    << "\ntimestamp_chunk       : " << statistics.timestamp_chunk
    << std::endl;
}

inline
static void show_metadata(FileToGraphic const & player) {
    std::cout
    << "\nWRM file version      : " << player.info_version
    << "\nWidth                 : " << player.info_width
    << "\nHeight                : " << player.info_height
    << "\nBpp                   : " << player.info_bpp
    << "\nCache 0 entries       : " << player.info_cache_0_entries
    << "\nCache 0 size          : " << player.info_cache_0_size
    << "\nCache 1 entries       : " << player.info_cache_1_entries
    << "\nCache 1 size          : " << player.info_cache_1_size
    << "\nCache 2 entries       : " << player.info_cache_2_entries
    << "\nCache 2 size          : " << player.info_cache_2_size
    << '\n';

    if (player.info_version > 3) {
        //cout << "Cache 3 entries       : " << player.info_cache_3_entries                         << endl;
        //cout << "Cache 3 size          : " << player.info_cache_3_size                            << endl;
        //cout << "Cache 4 entries       : " << player.info_cache_4_entries                         << endl;
        //cout << "Cache 4 size          : " << player.info_cache_4_size                            << endl;
        std::cout << "Compression algorithm : " << static_cast<int>(player.info_compression_algorithm) << '\n';
    }
    std::cout.flush();
}

inline
static void raise_error(std::string const & output_filename, int code, const char * message, uint32_t verbose) {
    if (!output_filename.length()) {
        return;
    }

    char outfile_pid[32];
    snprintf(outfile_pid, sizeof(outfile_pid), "%06u", unsigned(getpid()));

    char outfile_path     [1024] = {};
    char outfile_basename [1024] = {};
    char outfile_extension[1024] = {};

    canonical_path( output_filename.c_str()
                  , outfile_path
                  , sizeof(outfile_path)
                  , outfile_basename
                  , sizeof(outfile_basename)
                  , outfile_extension
                  , sizeof(outfile_extension)
                  , verbose
                  );

    char progress_filename[4096];
    snprintf( progress_filename, sizeof(progress_filename), "%s%s-%s.pgs"
            , outfile_path, outfile_basename, outfile_pid);

    UpdateProgressData update_progress_data(progress_filename, 0, 0, 0, 0);

    update_progress_data.raise_error(code, message);
}

template<class CaptureMaker, class... ExtraArguments>
static int do_record( Transport & in_wrm_trans, const timeval begin_record, const timeval end_record
                    , const timeval begin_capture, const timeval end_capture, std::string const & output_filename
                    , int capture_bpp, int wrm_compression_algorithm_
                    , Inifile & ini, Random & rnd, unsigned file_count, uint32_t order_count, uint32_t clear, unsigned zoom
                    , unsigned png_width, unsigned png_height
                    , bool show_file_metadata, bool show_statistics, uint32_t verbose
                    , ExtraArguments && ... extra_argument) {
    for (unsigned i = 1; i < file_count ; i++) {
        in_wrm_trans.next();
    }

    FileToGraphic player(&in_wrm_trans, begin_capture, end_capture, false, verbose);

    if (show_file_metadata) {
        show_metadata(player);
        std::cout << "Duration (in seconds) : " << (end_record.tv_sec - begin_record.tv_sec + 1) << std::endl;
        if (!show_statistics && !output_filename.length()) {
            return 0;
        }
    }

    player.max_order_count = order_count;

    int return_code = 0;

    if (output_filename.length()) {
//        char outfile_pid[32];
//        snprintf(outfile_pid, sizeof(outfile_pid), "%06u", getpid());

        char outfile_path     [1024] = {};
        char outfile_basename [1024] = {};
        char outfile_extension[1024] = {};

        canonical_path( output_filename.c_str()
                      , outfile_path
                      , sizeof(outfile_path)
                      , outfile_basename
                      , sizeof(outfile_basename)
                      , outfile_extension
                      , sizeof(outfile_extension)
                      , verbose
                      );

        if (verbose) {
//            std::cout << "Output file path: " << outfile_path << outfile_basename << '-' << outfile_pid << outfile_extension <<
            std::cout << "Output file path: " << outfile_path << outfile_basename << outfile_extension <<
                '\n' << std::endl;
        }

        if (clear == 1) {
            clear_files_flv_meta_png(outfile_path, outfile_basename);
        }

//        if (ini.get<cfg::video::wrm_compression_algorithm>() == USE_ORIGINAL_COMPRESSION_ALGORITHM) {
//            ini.set<cfg::video::wrm_compression_algorithm>(player.info_compression_algorithm);
//        }
        ini.set<cfg::video::wrm_compression_algorithm>(
            ((wrm_compression_algorithm_ == USE_ORIGINAL_COMPRESSION_ALGORITHM) ?
             player.info_compression_algorithm :
             wrm_compression_algorithm_));

//        if (ini.get<cfg::video::wrm_color_depth_selection_strategy>() == USE_ORIGINAL_COLOR_DEPTH) {
//            ini.set<cfg::video::wrm_color_depth_selection_strategy>(player.info_bpp);
//        }
        if (capture_bpp == USE_ORIGINAL_COLOR_DEPTH) {
            capture_bpp = player.info_bpp;
        }
//        ini.set<cfg::video::wrm_color_depth_selection_strategy>(capture_bpp);

        {
            CaptureMaker capmake( ((player.record_now.tv_sec > begin_capture.tv_sec) ? player.record_now : begin_capture)
                                , player.screen_rect.cx, player.screen_rect.cy
                                , player.info_bpp, capture_bpp, outfile_path, outfile_basename, outfile_extension
                                , ini, rnd, clear, verbose, std::forward<ExtraArguments>(extra_argument)...);
            auto & capture = capmake.capture;

            if (capture.capture_png) {
                if (png_width && png_height) {
                    auto get_percent = [](unsigned target_dim, unsigned source_dim) -> unsigned {
                        return ((target_dim * 100 / source_dim) + ((target_dim * 100 % source_dim) ? 1 : 0));
                    };
                    zoom = std::max<unsigned>(
                            get_percent(png_width, player.screen_rect.cx),
                            get_percent(png_height, player.screen_rect.cy)
                        );
                    //std::cout << "zoom: " << zoom << '%' << std::endl;
                }

                capture.psc->zoom(zoom);
            }
            player.add_consumer(&capture, &capture);

            char progress_filename[4096];
            snprintf( progress_filename, sizeof(progress_filename), "%s%s.pgs"
                    , outfile_path, outfile_basename);

            UpdateProgressData update_progress_data(
                progress_filename, begin_record.tv_sec, end_record.tv_sec, begin_capture.tv_sec, end_capture.tv_sec
            );

            if (update_progress_data.is_valid()) {
                try {
                    player.play(std::ref(update_progress_data), program_requested_to_shutdown);

                    if (program_requested_to_shutdown) {
                        update_progress_data.raise_error(65537, "Program requested to shutdown");
                    }
                }
                catch (Error const & e) {
                    const bool msg_with_error_id = false;
                    update_progress_data.raise_error(e.id, e.errmsg(msg_with_error_id));

                    return_code = -1;
                }
                catch (...) {
                    update_progress_data.raise_error(65536, "Unknown error");

                    return_code = -1;
                }
            }
            else {
                return_code = -1;
            }
        }

        if (!return_code && program_requested_to_shutdown) {
            clear_files_flv_meta_png(outfile_path, outfile_basename, verbose);
        }
    }
    else {
        try {
            player.play(program_requested_to_shutdown);
        }
        catch (Error const &) {
            return_code = -1;
        }
    }

    if (show_statistics && return_code == 0) {
      ::show_statistics(player.statistics);
    }

    return return_code;
}   // do_record

#endif
