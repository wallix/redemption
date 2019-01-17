#include "utils/stream.hpp"
#include "utils/sugar/unique_fd.hpp"

#include <array>
#include <bitset>
#include <iostream>
#include <iterator>
#include <fstream>
#include <vector>

#include <cerrno>
#include <cstring>

#include <unistd.h>

int main(int ac, char** av)
{
    if (ac < 2) {
        std::cerr << av[0] << " in.fv1 out.rbf\n";
        return 1;
    }

    unique_fd ufdr(::open(av[1], O_RDONLY));
    if (!ufdr) {
        std::cerr << av[0] << " " << av[1] << " " << strerror(errno) << "\n";
        return 2;
    }

    std::filebuf out;
    if (!out.open(av[2], std::ios::binary | std::ios::out | std::ios::trunc)) {
        std::cerr << av[0] << " " << av[2] << " " << strerror(errno) << "\n";
        return 3;
    }
    std::vector<uint8_t> data_out_file;

    uint8_t buf[1024*4];
    char const* const cbuf = reinterpret_cast<char*>(buf);
    InStream stream(buf, 0);

    using std::begin;
    using std::end;

    bool is_eof = false;

    auto read = [&](size_t n){
        if (stream.in_remain() < n) {
            memmove(buf, stream.get_current(), stream.in_remain());
            ssize_t r = 0;
            uint8_t* first = begin(buf) + stream.in_remain();
            uint8_t* last = end(buf);
            do {
                r = ::read(ufdr.fd(), first, last - first);
                first += r;
            } while ((first - buf < n && r) || (r == -1 ? errno == EINTR : false));

            if (r == -1) {
                throw std::runtime_error(strerror(errno));
            }

            if (r == 0 || first - buf < n) {
                if (r == 0) {
                    is_eof = true;
                }
                throw std::runtime_error("no more data");
            }

            stream = InStream({buf, first});
        }
    };

    auto const header_sig_sz = 4;
    auto const header_data_sz = 32 + 2 + 2;
    auto const header_padding_sz = 8;
    auto const header_sz = header_sig_sz + header_data_sz + header_padding_sz;

    read(header_sz);
    char const* format_name = "FNT1";
    if (0 != strncmp(reinterpret_cast<char*>(buf), format_name, 4)) {
        throw std::runtime_error(std::string("no ") + format_name + " format: " + std::string(cbuf, 4));
    }

    data_out_file.insert(end(data_out_file), "RBF1", "RBF1"+4);
    data_out_file.insert(end(data_out_file), cbuf + header_sig_sz, cbuf + header_sig_sz + header_data_sz);
    stream.in_skip_bytes(header_sz);

    size_t pos_header = 0;

    size_t MAX_GLYPHS = 0x4e00;

    {
        uint8_t obuf[64];
        OutStream out_stream{obuf};
        out_stream.out_uint16_le(16);
        out_stream.out_uint32_le(MAX_GLYPHS - 32);
        out_stream.out_uint32_le(254160);
        pos_header = data_out_file.size();
        data_out_file.insert(end(data_out_file), obuf, obuf+10);
    }

    long long total = 0;
    uint16_t height_max = 0;

    size_t index = 32;
    for (; index < MAX_GLYPHS ; ++index) {
        try {
            read(16);
        }
        catch (std::exception const&) {
            if (is_eof) {
                break;
            }
            throw;
        }

        auto const width = stream.in_uint16_le();
        auto const height = stream.in_uint16_le();
        auto const baseline = stream.in_sint16_le();
        auto const offset = stream.in_sint16_le();
        auto const incby = stream.in_sint16_le();
        stream.in_skip_bytes(6);

        auto const data_len = align4(nbbytes(width) * height);
        read(data_len);
        auto data = stream.get_current();
        auto const data_start = data;
        stream.in_skip_bytes(data_len);

        std::cout << index << " (" << char(index) << ") " << width << " " << height << " " << baseline << " " << offset << " " << incby << " " << data_len << "\n";

        height_max = std::max(height_max, height);

        int min_line = -1;
        int max_line = -1;
        int min_col = 64;
        int max_col = -1;
        for (int y = 0; y < height; ++y) {
            min_col = std::min(
                min_col,
                (*data & 0b10000000) ? 0 :
                (*data & 0b01000000) ? 1 :
                (*data & 0b00100000) ? 2 :
                (*data & 0b00010000) ? 3 :
                (*data & 0b00001000) ? 4 :
                (*data & 0b00000100) ? 5 :
                (*data & 0b00000010) ? 6 :
                (*data & 0b00000001) ? 7 : 8);
            max_col = std::max(
                max_col,
                (data[width/8-1] & 0b00000001) ? 8 :
                (data[width/8-1] & 0b00000010) ? 7 :
                (data[width/8-1] & 0b00000100) ? 6 :
                (data[width/8-1] & 0b00001000) ? 5 :
                (data[width/8-1] & 0b00010000) ? 4 :
                (data[width/8-1] & 0b00100000) ? 3 :
                (data[width/8-1] & 0b01000000) ? 2 :
                (data[width/8-1] & 0b10000000) ? 1 : 0);
            int has_value = 0;
            for (int x = 0; x < width/8; ++x) {
                has_value |= *data;
                std::cout << std::bitset<8>(*data++);
            }
            std::cout << (has_value ? " -\n" : " x\n");
            if (has_value) {
                if (min_line == -1) {
                    min_line = y;
                }
                max_line = y;
            }
        }
        max_col = 8 - max_col;
        std::cout << min_line << " " << max_line << " " << min_col << " " << max_col << "\n";
        std::cout << "\n";

        uint8_t obuf[64];
        OutStream out_stream{obuf};

        if (min_line == -1) {
            out_stream.out_uint32_le(index);

            out_stream.out_uint16_le(0);
            out_stream.out_uint16_le(0);

            out_stream.out_uint16_le(0);
            out_stream.out_uint16_le(incby);
            out_stream.out_uint16_le(0);

            out_stream.out_uint16_le(1);
            out_stream.out_uint16_le(1);

            out_stream.out_copy_bytes("\0\0\0\0", 4);
            total += 4;

            data_out_file.insert(end(data_out_file), out_stream.get_data(), out_stream.get_current());
            continue;
        }

        data = data_start + width/8 * min_line;
        auto const new_height = max_line - min_line + 1;
        auto const new_width = width - min_col - max_col;

        out_stream.out_uint32_le(index);

        out_stream.out_uint16_le(offset);
        out_stream.out_uint16_le(min_line);

        out_stream.out_uint16_le(0);
        out_stream.out_uint16_le(incby-offset);
        out_stream.out_uint16_le(0);

        out_stream.out_uint16_le(new_width);
        out_stream.out_uint16_le(new_height);
        data_out_file.insert(end(data_out_file), out_stream.get_data(), out_stream.get_current());

        long long total_buf = total;

        for (int y = 0; y < new_height; ++y) {
            unsigned long long ull = 0;
            for (int x = 0; x < width/8; ++x) {
                ull = (ull << 8) + *data++;
            }
            ull >>= max_col;
            // assert(max_col >= 0);
            // assert(max_col < 8);

            constexpr size_t Nbits = 128;
            auto const line = std::bitset<Nbits>(ull).to_string().substr(Nbits-new_width);
            std::cout << line << "\n";

            int counter = 0;
            uint8_t data_bits = 0;
            for (auto c : line) {
                data_bits = (data_bits << 1) + (c == '1');
                if (++counter == 8) {
                    data_out_file.emplace_back(data_bits);
                    ++total;
                    data_bits = 0;
                    counter = 0;
                }
            }
            if (counter) {
                data_bits <<= (8 - counter);
                data_out_file.emplace_back(data_bits);
                ++total;
            }
        }

        auto const n = nbbytes(new_width) * new_height;
        auto const padding = align4(n) - n;
        auto* spad = "\0\0\0\0\0\0\0\0\0";
        data_out_file.insert(end(data_out_file), spad, spad+padding);
        total += padding;

        std::cout << index << " " << min_col << " " << min_line << " " << min_col << " " << new_width << " " << width-new_width << " " << new_width << " " << new_height << " " << total-total_buf << " " << padding << "\n\n\n";
    }

    OutStream out_stream{&data_out_file[pos_header], 10};
    out_stream.out_uint16_le(height_max + 1);
    out_stream.out_uint32_le(index - 32);
    out_stream.out_uint32_le(total);

    out.sputn(reinterpret_cast<char*>(data_out_file.data()), data_out_file.size());

    std::cout << height_max << " " << total;
}
