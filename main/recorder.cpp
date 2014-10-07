/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Product name: redemption, a FLOSS RDP proxy
 * Copyright (C) Wallix 2010-2013
 * Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 * Raphaël Zhou
 *
 * recorder main program
 *
 */

#include <iostream>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <utility>
#include <string>

#define LOGPRINT
#include "version.hpp"

#include "fileutils.hpp"
#include "in_meta_sequence_transport.hpp"
#include "capture.hpp"
#include "ChunkToFile.hpp"
#include "FileToChunk.hpp"
#include "FileToGraphic.hpp"

static int recompress_or_record( Transport & in_wrm_trans, const timeval begin_capture
                               , const timeval end_capture, uint32_t verbose
                               , std::string & output_filename, Inifile & ini
                               , unsigned file_count, uint32_t order_count, uint32_t clear, unsigned zoom
                               , bool show_file_metadata);

static const unsigned USE_ORIGINAL_COMPRESSION_ALGORITHM = 0xFFFFFFFF;
static const unsigned USE_ORIGINAL_COLOR_DEPTH           = 0xFFFFFFFF;

int main(int argc, char** argv)
{
    openlog("redrec", LOG_CONS | LOG_PERROR, LOG_USER);

    const char * copyright_notice =
    "\n"
    "ReDemPtion RECorder" VERSION ": An RDP movie converter.\n"
    "Copyright (C) Wallix 2010-2013.\n"
    "Christophe Grosjean, Jonathan Poelen and Raphaël Zhou\n"
    "\n"
    ;

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
    bool        auto_output_file   = false;
    bool        remove_input_file  = false;
    std::string wrm_compression_algorithm;  // output compression algorithm.
    std::string wrm_color_depth;

    boost::program_options::options_description desc("Options");
    desc.add_options()
    ("help,h", "produce help message")
    ("version,v", "show software version")
    ("output-file,o", boost::program_options::value(&output_filename), "output base filename (see --output-type)")
    ("input-file,i", boost::program_options::value(&input_filename), "input base filename (see --input-type)")

    ("begin,b", boost::program_options::value<uint32_t>(&begin_cap), "begin capture time (in seconds), default=none")
    ("end,e", boost::program_options::value<uint32_t>(&end_cap), "end capture time (in seconds), default=none")
    ("count,c", boost::program_options::value<uint32_t>(&order_count), "Number of orders to execute before stopping, default=0 execute all orders")

    ("png_limit,l", boost::program_options::value<uint32_t>(&png_limit), "maximum number of png files to create (remove older), default=10, 0 will disable png capture")
    ("png_interval,n", boost::program_options::value<uint32_t>(&png_interval), "time interval between png captures, default=60 seconds")

    ("frameinterval,r", boost::program_options::value<uint32_t>(&wrm_frame_interval), "time between consecutive capture frames (in 100/th of seconds), default=100 one frame per second")

    ("breakinterval,k", boost::program_options::value<uint32_t>(&wrm_break_interval), "number of seconds between splitting wrm files in seconds(default, one wrm every day)")

    ("png,p", "enable png capture")
    ("wrm,w", "enable wrm capture")
    ("clear", boost::program_options::value<uint32_t>(&clear), "Clear old capture files with same prefix (default on)")
    ("verbose", boost::program_options::value<uint32_t>(&verbose), "more logs")
    ("zoom", boost::program_options::value<uint32_t>(&zoom), "scaling factor for png capture (default 100%)")
    ("meta,m", "show file metadata")

    //("compression,z", boost::program_options::value(&wrm_compression_algorithm), "wrm compression algorithm (default=original, none, gzip, snappy, lzma)")
    ("compression,z", boost::program_options::value(&wrm_compression_algorithm), "wrm compression algorithm (default=original, none, gzip, snappy)")
    ("color-depth,d", boost::program_options::value(&wrm_color_depth),           "wrm color depth (default=original, 16, 24)")

    ("auto-output-file",  "Append suffix to input base filename to generate output base filename automatically")
    ("remove-input-file", "Remove input file")
    ;

    Inifile ini;

    try {
        boost::program_options::variables_map options;
        boost::program_options::store(
            boost::program_options::command_line_parser(argc, argv).options(desc)
    //            .positional(p)
                .run(),
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

        if (input_filename.c_str()[0] == 0){
            cout << "Missing input filename\n\n";
            cout << copyright_notice;
            cout << "Usage: redrec [options]\n\n";
            cout << desc << endl;
            exit(-1);
        }

        show_file_metadata = (options.count("meta"             ) > 0);
        auto_output_file   = (options.count("auto-output-file" ) > 0);
        remove_input_file  = (options.count("remove-input-file") > 0);

        if (!show_file_metadata && !auto_output_file && (output_filename.c_str()[0] == 0)) {
            cout << "Missing output filename\n\n";
            cout << copyright_notice;
            cout << "Usage: redrec [options]\n\n";
            cout << desc << endl;
            exit(-1);
        }

        if (output_filename.c_str()[0] && auto_output_file) {
            cout << "Conflicting options : --output-file and --auto-output-file\n\n";
            exit(-1);
        }

        if (options.count("compression") > 0) {
                 if (0 == strcmp(wrm_compression_algorithm.c_str(), "none"     )) {
                ini.video.wrm_compression_algorithm = 0;
            }
            else if (0 == strcmp(wrm_compression_algorithm.c_str(), "gzip"     )) {
                ini.video.wrm_compression_algorithm = 1;
            }
            else  if (0 == strcmp(wrm_compression_algorithm.c_str(), "snappy"  )) {
                ini.video.wrm_compression_algorithm = 2;
            }
            //else  if (0 == strcmp(wrm_compression_algorithm.c_str(), "lzma"    )) {
            //    ini.video.wrm_compression_algorithm = 3;
            //}
            else  if (0 == strcmp(wrm_compression_algorithm.c_str(), "original")) {
                ini.video.wrm_compression_algorithm = USE_ORIGINAL_COMPRESSION_ALGORITHM;
            }
            else {
                cout << "Unknown wrm compression algorithm\n\n";
                exit(-1);
            }
        }
        else {
            ini.video.wrm_compression_algorithm = USE_ORIGINAL_COMPRESSION_ALGORITHM;
        }

        if (options.count("color-depth") > 0) {
                 if (0 == strcmp(wrm_color_depth.c_str(), "16"       )) {
                ini.video.wrm_color_depth_selection_strategy = 16;
            }
            else if (0 == strcmp(wrm_color_depth.c_str(), "24"       )) {
                ini.video.wrm_color_depth_selection_strategy = 24;
            }
            else  if (0 == strcmp(wrm_color_depth.c_str(), "original")) {
                ini.video.wrm_color_depth_selection_strategy = USE_ORIGINAL_COLOR_DEPTH;
            }
            else {
                cout << "Unknown wrm color depth\n\n";
                exit(-1);
            }
        }
        else {
            ini.video.wrm_color_depth_selection_strategy = USE_ORIGINAL_COLOR_DEPTH;
        }

        ini.video.png_limit = png_limit;
        ini.video.png_interval = png_interval;
        ini.video.frame_interval = wrm_frame_interval;
        ini.video.break_interval = wrm_break_interval;
        ini.video.capture_wrm = options.count("wrm") > 0;
        ini.video.capture_png = (options.count("png") > 0);

        if (   output_filename.length()
            && !(ini.video.capture_png | ini.video.capture_wrm)) {
            cout << "Missing target format : need --png or --wrm\n\n";
            exit(-1);
        }
    }
    catch(boost::program_options::error& error) {
        cout << error.what() << endl;
        cout << copyright_notice;
        cout << "Usage: redrec [options]\n\n";
        cout << desc << endl;
        exit(-1);
    };

    timeval begin_capture;
    begin_capture.tv_sec = begin_cap; begin_capture.tv_usec = 0;
    timeval end_capture;
    end_capture.tv_sec = end_cap; end_capture.tv_usec = 0;

    char infile_path[1024];
    char infile_basename[1024];
    char infile_extension[128];
    char infile_prefix[4096];

    strcpy(infile_path,      "./"          );   // default value, actual one should come from output_filename
    strcpy(infile_basename,  "redrec_input");   // default value actual one should come from output_filename
    strcpy(infile_extension, ".mwrm"       );
    canonical_path( input_filename.c_str()
                  , infile_path
                  , sizeof(infile_path)
                  , infile_basename
                  , sizeof(infile_basename)
                  , infile_extension
                  , sizeof(infile_extension)
                  );
    if (verbose) {
        cout << endl << "Input file path: " << infile_path << infile_basename << infile_extension <<
            endl;
    }

    infile_prefix[0] = 0;
    sprintf(infile_prefix, "%s%s", infile_path, infile_basename);

    if (auto_output_file) {
        output_filename =  infile_path;
        output_filename += infile_basename;
        output_filename += "-redrec";
        output_filename += infile_extension;

        cout << endl << "Output file path (autogenerated): " << output_filename << endl;
    }

    TODO("before continuing to work with input file, check if it's mwrm or wrm and use right object in both cases")

    TODO("also check if it contains any wrm at all and at wich one we should start depending on input time")
    TODO("if start and stop time are outside wrm, users should also be warned")


    unsigned count = 0;
    try {
        InMetaSequenceTransport in_wrm_trans_tmp(infile_prefix, infile_extension);
        in_wrm_trans_tmp.next();
        TODO("a negative time should be a time relative to end of movie")
        REDOC("less than 1 year means we are given a time relatve to beginning of movie")
        if (begin_cap < 31536000){ // less than 1 year, it is relative not absolute timestamp
            // begin_capture.tv_usec is 0
            begin_cap += in_wrm_trans_tmp.begin_chunk_time();
        }
        if (end_cap < 31536000){ // less than 1 year, it is relative not absolute timestamp
            // begin_capture.tv_usec is 0
            end_cap += in_wrm_trans_tmp.begin_chunk_time();
        }
        while (begin_cap >= in_wrm_trans_tmp.end_chunk_time()){
            in_wrm_trans_tmp.next();
        }
        count = in_wrm_trans_tmp.get_seqno();
    }
    catch (const Error & e) {
        if (e.id == static_cast<unsigned>(ERR_TRANSPORT_NO_MORE_DATA)){
            printf("Asked time not found in mwrm file\n");
        };
        exit(-1);
    };

    InMetaSequenceTransport in_wrm_trans(infile_prefix, infile_extension);
/*
    for (unsigned i = 1; i < count ; i++){
        in_wrm_trans.next();
    }

    FileToGraphic player(&in_wrm_trans, begin_capture, end_capture, false, verbose);
    if (show_file_metadata) {
        cout << endl;
        cout << "WRM file version      : " << player.info_version         << endl;
        cout << "Width                 : " << player.info_width           << endl;
        cout << "Height                : " << player.info_height          << endl;
        cout << "Bpp                   : " << player.info_bpp             << endl;
        cout << "Cache 0 entries       : " << player.info_cache_0_entries << endl;
        cout << "Cache 0 size          : " << player.info_cache_0_size    << endl;
        cout << "Cache 1 entries       : " << player.info_cache_1_entries << endl;
        cout << "Cache 1 size          : " << player.info_cache_1_size    << endl;
        cout << "Cache 2 entries       : " << player.info_cache_2_entries << endl;
        cout << "Cache 2 size          : " << player.info_cache_2_size    << endl;
        if (player.info_version > 3) {
            //cout << "Cache 3 entries       : " << player.info_cache_3_entries                         << endl;
            //cout << "Cache 3 size          : " << player.info_cache_3_size                            << endl;
            //cout << "Cache 4 entries       : " << player.info_cache_4_entries                         << endl;
            //cout << "Cache 4 size          : " << player.info_cache_4_size                            << endl;
            cout << "Compression algorithm : " << static_cast<int>(player.info_compression_algorithm) << endl;
        }
        cout << endl;

        if (!output_filename.length()) {
            return 0;
        }
    }
    player.max_order_count = order_count;

    const char * outfile_fullpath = output_filename.c_str();
    char outfile_path[1024];
    char outfile_basename[1024];
    char outfile_extension[128];
    strcpy(outfile_path, "./"); // default value, actual one should come from output_filename
    strcpy(outfile_basename, "redrec_output"); // default value actual one should come from output_filename
    strcpy(outfile_extension, "");
    canonical_path(outfile_fullpath,
        outfile_path, sizeof(outfile_path),
        outfile_basename, sizeof(outfile_basename),
        outfile_extension, sizeof(outfile_extension));
    if (clear == 1) {
        clear_files_flv_meta_png(outfile_path, outfile_basename);
    }

    Capture capture( player.record_now, player.screen_rect.cx, player.screen_rect.cy, player.info_bpp, 24
                   , outfile_path, outfile_path, ini.video.hash_path, outfile_basename, false
                   , false, NULL, ini);
    if (capture.capture_png){
        capture.psc->zoom(zoom);
    }
    player.add_consumer((RDPGraphicDevice * )&capture, (RDPCaptureDevice * )&capture);

    int return_code = 0;
    try {
        player.play();
    }
    catch (Error e) {
        return_code = -1;
    }

    return return_code;
*/

    int result = recompress_or_record( in_wrm_trans, begin_capture, end_capture, verbose
                                     , output_filename, ini, count, order_count, clear, zoom
                                     , show_file_metadata);

    if (!result && remove_input_file) {
        InMetaSequenceTransport in_wrm_trans_tmp(infile_prefix, infile_extension);


        std::vector<std::string> files;

        char infile_fullpath[2048];
        snprintf(infile_fullpath, sizeof(infile_fullpath), "%s%s%s", infile_path, infile_basename, infile_extension);
        files.push_back(infile_fullpath);

        try {
            do {
                in_wrm_trans.next();
                files.push_back(in_wrm_trans.path());
            }
            while (true);
        }
        catch (const Error & e){
            if (e.id != ERR_TRANSPORT_NO_MORE_DATA) {
                throw;
            }
        };

        cout << endl;
        for (std::vector<std::string>::reverse_iterator rit = files.rbegin(); rit != files.rend(); ++rit) {
            unlink(rit->c_str());
            cout << "Removed : " << *rit << endl;
        }
    }

    cout << endl;

    return result;
}

static int do_recompress(Transport & in_wrm_trans, std::string & output_filename, Inifile & ini, uint32_t verbose) {
    FileToChunk player(&in_wrm_trans, 0);

    char outfile_path[1024];
    char outfile_basename[1024];
    char outfile_extension[1024];

    strcpy(outfile_path,      PNG_PATH "/");    // default value, actual one should come from output_filename
    strcpy(outfile_basename,  "redrec_output"); // default value, actual one should come from output_filename
    strcpy(outfile_extension, "");              // extension is ignored for targets anyway

    const char * outfile_fullpath = output_filename.c_str();

    canonical_path( outfile_fullpath
                  , outfile_path
                  , sizeof(outfile_path)
                  , outfile_basename
                  , sizeof(outfile_basename)
                  , outfile_extension
                  , sizeof(outfile_extension)
                  , verbose
                  );

    if (verbose) {
        cout << "Output file path: " << outfile_path << outfile_basename << outfile_extension <<
            endl << endl;
    }

    if (recursive_create_directory(outfile_path, S_IRWXU|S_IRGRP|S_IXGRP, ini.video.capture_groupid) != 0) {
        cerr << "Failed to create directory: \"" << outfile_path << "\"" << endl;
    }

    struct timeval now = tvtime();

    if (ini.video.wrm_compression_algorithm == USE_ORIGINAL_COMPRESSION_ALGORITHM) {
        ini.video.wrm_compression_algorithm = player.info_compression_algorithm;
    }

    int return_code = 0;
    try {
        unique_ptr<Transport> wrm_trans;
        CryptoContext         cctx;

        if (ini.globals.enable_file_encryption.get()) {
            memset(&cctx, 0, sizeof(cctx));
            memcpy(cctx.crypto_key, ini.crypto.key0, sizeof(cctx.crypto_key));
            memcpy(cctx.hmac_key,   ini.crypto.key1, sizeof(cctx.hmac_key  ));

            wrm_trans.reset(
                new CryptoOutMetaSequenceTransport( &cctx, outfile_path, ini.video.hash_path, outfile_basename, now
                                                  , player.info_width, player.info_height, ini.video.capture_groupid)
                );
        }
        else {
            wrm_trans.reset(
                new OutMetaSequenceTransport( outfile_path, outfile_basename, now
                                            , player.info_width, player.info_height, ini.video.capture_groupid)
                );
        }

        ChunkToFile recorder( wrm_trans.get()

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

        player.play();
    }
    catch (...) {
        return_code = -1;
    }

    return return_code;
}

static int do_record( Transport & in_wrm_trans, const timeval begin_capture
                    , const timeval end_capture, uint32_t verbose
                    , std::string & output_filename, Inifile & ini, uint32_t order_count, uint32_t clear
                    , unsigned zoom, bool show_file_metadata) {
    FileToGraphic player(&in_wrm_trans, begin_capture, end_capture, false, verbose);

    if (show_file_metadata) {
        cout << endl;
        cout << "WRM file version      : " << player.info_version         << endl;
        cout << "Width                 : " << player.info_width           << endl;
        cout << "Height                : " << player.info_height          << endl;
        cout << "Bpp                   : " << player.info_bpp             << endl;
        cout << "Cache 0 entries       : " << player.info_cache_0_entries << endl;
        cout << "Cache 0 size          : " << player.info_cache_0_size    << endl;
        cout << "Cache 1 entries       : " << player.info_cache_1_entries << endl;
        cout << "Cache 1 size          : " << player.info_cache_1_size    << endl;
        cout << "Cache 2 entries       : " << player.info_cache_2_entries << endl;
        cout << "Cache 2 size          : " << player.info_cache_2_size    << endl;
        if (player.info_version > 3) {
            //cout << "Cache 3 entries       : " << player.info_cache_3_entries                         << endl;
            //cout << "Cache 3 size          : " << player.info_cache_3_size                            << endl;
            //cout << "Cache 4 entries       : " << player.info_cache_4_entries                         << endl;
            //cout << "Cache 4 size          : " << player.info_cache_4_size                            << endl;
            cout << "Compression algorithm : " << static_cast<int>(player.info_compression_algorithm) << endl;
        }

        if (!output_filename.length()) {
            return 0;
        }
    }

    player.max_order_count = order_count;

    const char * outfile_fullpath = output_filename.c_str();
    char outfile_path[1024];
    char outfile_basename[1024];
    char outfile_extension[128];
    strcpy(outfile_path,      "./"           ); // default value, actual one should come from output_filename
    strcpy(outfile_basename,  "redrec_output"); // default value actual one should come from output_filename
    strcpy(outfile_extension, ""             ); // extension is ignored for targets anyway

    canonical_path( outfile_fullpath
                  , outfile_path
                  , sizeof(outfile_path)
                  , outfile_basename
                  , sizeof(outfile_basename)
                  , outfile_extension
                  , sizeof(outfile_extension)
                  );

    if (verbose) {
        cout << "Output file path: " << outfile_path << outfile_basename << outfile_extension <<
            endl << endl;
    }

    if (clear == 1) {
        clear_files_flv_meta_png(outfile_path, outfile_basename);
    }

    Capture capture( player.record_now, player.screen_rect.cx, player.screen_rect.cy, player.info_bpp, 24
                   , outfile_path, outfile_path, ini.video.hash_path, outfile_basename, false
                   , false, NULL, ini);
    if (capture.capture_png){
        capture.psc->zoom(zoom);
    }
    player.add_consumer((RDPGraphicDevice * )&capture, (RDPCaptureDevice * )&capture);

    int return_code = 0;
    try {
        player.play();
    }
    catch (Error e) {
        return_code = -1;
    }

    return return_code;
}

static int recompress_or_record( Transport & in_wrm_trans, const timeval begin_capture
                               , const timeval end_capture, uint32_t verbose
                               , std::string & output_filename, Inifile & ini
                               , unsigned file_count, uint32_t order_count, uint32_t clear, unsigned zoom
                               , bool show_file_metadata) {
    for (unsigned i = 1; i < file_count ; i++) {
        in_wrm_trans.next();
    }

    if (ini.video.capture_png || show_file_metadata ||
        (ini.video.wrm_color_depth_selection_strategy != USE_ORIGINAL_COLOR_DEPTH) || order_count) {
        if (verbose) {
            cout << "[A]"<< endl;
        }
        return do_record( in_wrm_trans, begin_capture, end_capture, verbose
                        , output_filename, ini, order_count, clear, zoom
                        , show_file_metadata);
    }
    else {
        if (verbose) {
            cout << "[R]" << endl;
        }
        return do_recompress(in_wrm_trans, output_filename, ini, verbose);
    }
}
