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
#include <map>

#include "recorder/wrm_recorder_option.hpp"
#include "wrm_recorder.hpp"
#include "recorder/parse_command_line.hpp"
#include "recorder/wrm_recorder_init.hpp"

struct ExtractOption
: WrmRecorderOption
{
    bool display_type_order;
    bool display_number_order;
    bool display_weight_order;
    bool display_time;

    ExtractOption();

    virtual int normalize_options();

private:
    void add_default_options();
};

namespace po = boost::program_options;

ExtractOption::ExtractOption()
: WrmRecorderOption()
, display_type_order(true)
, display_number_order(true)
, display_weight_order(true)
, display_time(true)
{
    this->add_default_options();
}

void ExtractOption::add_default_options()
{
    this->desc.add_options()
    ("display-type-order,o",   "display order")
    ("display-number-order,c", "display number of order")
    ("display-weight-order,w", "display octets of order")
    ("display-time-file,T",    "display micro seconds of capture and files")
    ;
}

int ExtractOption::normalize_options()
{
    if (int error = WrmRecorderOption::normalize_options()){
        return error;
    }

    typedef std::pair<const char *, bool&> pair_type;
    pair_type p[] = {
        pair_type("display-type-order",   this->display_type_order),
        pair_type("display-number-order", this->display_number_order),
        pair_type("display-weight-order", this->display_weight_order),
        pair_type("display-time-file",    this->display_time),
    };
    po::variables_map::iterator end = this->options.end();
    for (std::size_t n = 0; n < sizeof(p)/sizeof(p[0]); ++n) {
        if (this->options.find(p[n].first) != end)
            p[n].second = true;
    }

    return SUCCESS;
}

struct WrmInfo
{
    uint weight;
    uint number;
};

typedef std::map<uint, WrmInfo> WrmInfoOrder;


int main(int argc, char** argv)
{
    ExtractOption opt;

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

    WrmInfoOrder info_order;
    uint weight;
    RDPUnserializer& unserializer = recorder.reader;
    Stream& stream = unserializer.stream;
    uint16_t& remaining_order_count = unserializer.remaining_order_count;

    while (recorder.selected_next_order())
    {
        weight = stream.end - stream.p;

        switch (recorder.chunk_type())
        {
            case  WRMChunk::BREAKPOINT:
            {
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

                --remaining_order_count;

                while (nb_img)
                {
                    recorder.selected_next_order();
                    std::cout
                    << "\n\t\tnumber: " << remaining_order_count
                    << "\n\t\tsize: " << (stream.end - stream.p);
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
                    std::cout << "\n\timage size: " << (stream.end - stream.p);
                    recorder.ignore_chunks();
                }

                weight = 0;
                for (uint n = 0; n != 27; ++n){
                    recorder.selected_next_order();
                    weight += stream.end - stream.p;
                    recorder.ignore_chunks();
                }
                std::cout << "\n\tcache stamp size: " << weight << '\n';
            }
                break;
            case WRMChunk::META_FILE:
                std::cout << "META_FILE: number: " << remaining_order_count
                << ", weight: " << weight
                << ", filename: ";
                std::cout.write(reinterpret_cast<const char *>(stream.p + 4),
                                stream.in_uint32_le()) << '\n';
                recorder.ignore_chunks();
                break;
            case WRMChunk::TIME_START:
            {
                timeval tv = recorder.get_start_time_order();
                std::cout << "TIME_START: number: " << remaining_order_count
                << ", weight: " << weight
                << ", sec: " << tv.tv_sec << ", usec: " << tv.tv_usec << '\n';
                --remaining_order_count;
            }
                break;
            case WRMChunk::TIMESTAMP:
            {
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
                std::cout << "NEXT_FILE_ID: number: " << remaining_order_count
                << ", weight: " << weight;
                recorder.interpret_order();
                std::cout << ", idx: " << recorder.idx_file << '/' << recorder.meta().files.size() << ", open: " << recorder.meta().files[recorder.idx_file] << '\n';
            }
                break;
            default:
                std::cout << recorder.chunk_type() << ": number: " << remaining_order_count
                << ", weight: " << weight << '\n';
                recorder.ignore_chunks();
                break;
        }
    }

    return 0;
}
