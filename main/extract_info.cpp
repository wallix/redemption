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

#include "recorder/wrm_recorder_option.hpp"
#include "wrm_recorder.hpp"
#include "recorder/parse_command_line.hpp"
#include "recorder/wrm_recorder_init.hpp"
#include "meta_file.hpp"

struct WrmInfo
{
    uint weight;
    uint number;

    WrmInfo()
    : weight(0)
    , number(0)
    {}
};


int main(int argc, char** argv)
{
    WrmRecorderOption opt;

    if (!parse_command_line(opt, argc, argv)){
        return 0;
    }

    InputType::enum_t itype;
    if (int error = opt.prepare(itype)){
        return error;
    }

    WRMRecorder recorder;
    if (int error = wrm_recorder_init(recorder, opt, itype)){
        return error;
    }

    {
        const DataMetaFile& meta = recorder.meta();
        std::cout << "meta v" << meta.version
        << "\nwidth: " << meta.width
        << "\nheight: " << meta.height
        << "\nfiles:";
        for (uint i = 0; i != meta.files.size(); ++i){
            std::cout << "\n\t" << (i+1) << ' ' << meta.files[i];
        }
        std::cout << "\n\n";
    }

    uint weight;
    RDPUnserializer& unserializer = recorder.reader;
    Stream& stream = unserializer.stream;
    uint16_t& remaining_order_count = unserializer.remaining_order_count;

    WrmInfo timestamp_info;
    WrmInfo breakpoint_info;
    WrmInfo meta_file_info;
    WrmInfo next_file_id_info;
    WrmInfo time_start_info;
    WrmInfo draw_info;
    WrmInfo other_info;

    while (recorder.selected_next_order())
    {
        weight = stream.end - stream.p;

        switch (recorder.chunk_type())
        {
            case  WRMChunk::BREAKPOINT:
            {
                uint start_weight = breakpoint_info.weight;
                ++breakpoint_info.number;
                uint16_t width = stream.in_uint16_le();
                uint16_t height = stream.in_uint16_le();
                /*uint8_t bpp = */stream.in_uint8();
                uint64_t sec = stream.in_uint64_le();
                uint64_t usec = stream.in_uint64_le();
                //unserializer.wait_cap.timer.sec() = sec;
                //unserializer.wait_cap.timer.usec() = usec;

                uint nb_img = stream.in_uint16_le() * stream.in_uint16_le();

                std::cout << "BREAKPOINT: number: " << remaining_order_count
                << "\n\twidth: " << width << ", height: " << height
                << "\n\tsec: " << sec << ", usec: " << usec
                << "\n\tweight: " << weight
                << "\n\timages: " << nb_img;
                breakpoint_info.weight += weight;

                --remaining_order_count;

                while (nb_img)
                {
                    recorder.selected_next_order();
                    weight = (stream.end - stream.p);
                    breakpoint_info.weight += weight;
                    std::cout
                    << "\n\t\tnumber: " << remaining_order_count
                    << "\n\t\tsize: " << weight;
                    nb_img -= remaining_order_count;
                    recorder.ignore_chunks();
                }

                recorder.selected_next_order();

                std::cout << "\n\torder size: " << (stream.end - stream.p);
                stream.p = stream.end - 2;
                nb_img = stream.in_uint16_le();

                remaining_order_count = 0;
                for (; nb_img; --nb_img){
                    recorder.selected_next_order();
                    weight = (stream.end - stream.p);
                    breakpoint_info.weight += weight;
                    std::cout << "\n\timage size: " << weight;
                    recorder.ignore_chunks();
                }

                weight = 0;
                for (uint n = 0; n != 27; ++n){
                    recorder.selected_next_order();
                    weight += stream.end - stream.p;
                    recorder.ignore_chunks();
                }
                breakpoint_info.weight += weight;
                std::cout << "\n\tcache stamp size: " << weight
                << "\n\ttotal: " << (breakpoint_info.weight - start_weight) << '\n';
            }
                break;
            case WRMChunk::META_FILE:
                ++meta_file_info.number;
                meta_file_info.weight += weight;
                std::cout << "META_FILE: number: " << remaining_order_count
                << ", weight: " << weight
                << ", filename: ";
                std::cout.write(reinterpret_cast<const char *>(stream.p + 4),
                                stream.in_uint32_le()) << '\n';
                recorder.ignore_chunks();
                break;
            case WRMChunk::TIME_START:
            {
                ++time_start_info.number;
                time_start_info.weight += weight;
                timeval tv = recorder.get_start_time_order();
                std::cout << "TIME_START: number: " << remaining_order_count
                << ", weight: " << weight
                << ", sec: " << tv.tv_sec << ", usec: " << tv.tv_usec << '\n';
                --remaining_order_count;
            }
                break;
            case WRMChunk::TIMESTAMP:
            {
                ++timestamp_info.number;
                timestamp_info.weight += weight;
                uint64_t micro_sec = stream.in_uint64_be();
                std::cout << "TIMESTAMP: number: " << remaining_order_count
                << ", weight: " << weight
                << ", micro seconde: " << micro_sec << '\n';
                --remaining_order_count;
                //unserializer.wait_cap.timer.sec() = sec;
                //unserializer.wait_cap.timer.usec() = usec;
            }
                break;
            case WRMChunk::NEXT_FILE_ID:
            {
                ++next_file_id_info.number;
                next_file_id_info.weight += weight;
                std::cout << "NEXT_FILE_ID: number: " << remaining_order_count
                << ", weight: " << weight;
                recorder.interpret_order();
                std::cout << ", idx: " << recorder.idx_file << '/' << recorder.meta().files.size() << ", open: " << recorder.meta().files[recorder.idx_file] << '\n';
            }
                break;
            default:
                WrmInfo& info = recorder.chunk_type() ? other_info : draw_info;
                ++info.number;
                info.weight += weight;
                std::cout << recorder.chunk_type() << ": number: " << remaining_order_count
                << ", weight: " << weight << '\n';
                recorder.ignore_chunks();
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
    ;

    return 0;
}
