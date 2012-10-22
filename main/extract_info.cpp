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
 * Copyright (C) Wallix 2010-2012
 * Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 *
 * recorder main program
 *
 */

#define LOGPRINT

#include <iostream>
#include <string>

#include "recorder/wrm_recorder_option.hpp"

#include "meta_file.hpp"


struct WrmInfo
{
    ulong weight;
    uint number;

    WrmInfo()
    : weight(0)
    , number(0)
    {}
};


int main(int argc, char** argv)
{
    
    WrmRecorderOption opt;
    if (!opt.parse_command_line(argc, argv)){
        printf("parse command line failed\n");
        return 0;
    }
    
    InputType::enum_t itype;
    timeval now;
    gettimeofday(&now, NULL);

    const char * filename = opt.in_filename.c_str();;
    printf("WRMRecorder opening file : %s", filename);
    int fd = ::open(filename, O_RDONLY);
    if (-1 == fd){
       printf("Error opening wrm reader file : %s", strerror(errno));
       exit(-1);
    }

    InFileTransport trans(fd);
    FileToGraphic reader(&trans, now, Rect());

    WRMRecorder recorder(now,
                        trans,
                        reader,
                        itype, 
                        opt.base_path, 
                        opt.ignore_dir_for_meta_in_wrm,
                        opt.times_in_meta_are_false,
                        opt.force_interpret_breakpoint,
                        opt.range,
                        opt.in_filename,
                        opt.idx_start);



    ulong weight;
    Stream& stream = reader.stream;
    uint16_t& remaining_order_count = reader.remaining_order_count;

    WrmInfo timestamp_info;
    WrmInfo breakpoint_info;
    WrmInfo meta_file_info;
    WrmInfo next_file_id_info;
    WrmInfo time_start_info;
    WrmInfo draw_info;
    WrmInfo other_info;

    while (reader.next_order())
    {
        weight = stream.end - stream.p;
        switch (reader.chunk_type)
        {
//            case  WRMChunk::BREAKPOINT:
//            {
//                ulong start_weight = breakpoint_info.weight;
//                ++breakpoint_info.number;
//                uint16_t width = stream.in_uint16_le();
//                uint16_t height = stream.in_uint16_le();
//                /*uint8_t bpp = */stream.in_uint8();
//                uint64_t sec = stream.in_uint64_le();
//                uint64_t usec = stream.in_uint64_le();
//                //unserializer.wait_cap.timer.sec() = sec;
//                //unserializer.wait_cap.timer.usec() = usec;

//                std::cout << "BREAKPOINT: number: " << remaining_order_count
//                << "\n\twidth: " << width << ", height: " << height
//                << "\n\tsec: " << sec << ", usec: " << usec << '\n';
//                breakpoint_info.weight += weight;

//                --remaining_order_count;

//                reader.selected_next_order();

//                weight = stream.end - stream.p;
//                std::cout << "\torder size: " << weight;
//                breakpoint_info.weight += weight;

//                remaining_order_count = 0;
//                std::cout << "\n\tcache: \n";
//                while (1)
//                {
//                    stream.init(14);
//                    trans.recv(&stream.end, 14);
//                    uint16_t idx = stream.in_uint16_le();
//                    stream.p += 4;
//                    uint16_t cx = stream.in_uint16_le();
//                    uint16_t cy = stream.in_uint16_le();
//                    uint32_t buffer_size = stream.in_uint32_le();
//                    breakpoint_info.weight += 14 + buffer_size;
//                    if (idx == 8192 * 3 + 1){
//                        break;
//                    }
//                    std::cout << "\t\timage: width: " << cx << ", height: " << cy << ", size (zip compression): " << buffer_size << " B\n";
//                    stream.init(buffer_size);
//                    trans.recv(&stream.end, buffer_size);
//                }
//                std::cout << "\ttotal: " << (breakpoint_info.weight - start_weight) << '\n';
//            }
//                break;
            case WRMChunk::META_FILE:
            {
                reader.stream.p = reader.stream.end;
                reader.remaining_order_count = 0;

            }
                break;
            case WRMChunk::TIMESTAMP:
            {
                ++timestamp_info.number;
                timestamp_info.weight += weight;
                uint64_t micro_sec = stream.in_uint64_le();
                std::cout << "TIMESTAMP: number: " << remaining_order_count
                << ", size: " << weight
                << " B, usec: " << micro_sec << '\n';
                --remaining_order_count;
                //unserializer.wait_cap.timer.sec() = sec;
                //unserializer.wait_cap.timer.usec() = usec;
            }
                break;
            case WRMChunk::NEXT_FILE_ID:
            {
                ++next_file_id_info.number;
                next_file_id_info.weight += weight;
                uint16_t tmp = remaining_order_count;
                recorder.interpret_order();
                DataFile& info = reader.data_meta.files[recorder.idx_file];
                std::cout << "NEXT_FILE_ID: number: " << tmp
                << ", size: " << weight << " B, idx: "
                << recorder.idx_file << '/' << reader.data_meta.files.size()
                << ", open: wrm: " << info.wrm_filename
                << ", png: " << info.png_filename
                << ", start_sec: " << info.start_sec
                << ", start_usec: " << info.start_usec << '\n';
            }
                break;
            default:
                WrmInfo& info = reader.chunk_type ? other_info : draw_info;
                ++info.number;
                info.weight += weight;
                std::cout << reader.chunk_type << ": number: " << remaining_order_count
                << ", size: " << weight << " B\n";
                reader.stream.p = reader.stream.end;
                reader.remaining_order_count = 0;
                break;
        }
    }

    std::cout << '\n'
    << timestamp_info.number << " TIMESTAMP " << timestamp_info.weight << " B\n"
    << meta_file_info.number << " META_FILE " << meta_file_info.weight << " B\n"
    << breakpoint_info.number << " BREAKPOINT " << breakpoint_info.weight << " B\n"
    << next_file_id_info.number << " NEXT_FILE_ID " << next_file_id_info.weight << " B\n"
    << time_start_info.number << " TIME_START " << time_start_info.weight << " B\n"
    << draw_info.number << " type=0 " << draw_info.weight << " B\n"
    << other_info.number << " other " << other_info.weight << " B\n"
    << "total " << (
        timestamp_info.weight
        + meta_file_info.weight
        + breakpoint_info.weight
        + next_file_id_info.weight
        + time_start_info.weight
        + draw_info.weight
        + other_info.weight
    ) << " B\n";

    return 0;
}
