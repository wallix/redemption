/*
SPDX-FileCopyrightText: 2024 Wallix Proxies Team
SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <utility>
#include <charconv>
#include <stdexcept>

#include <cstring>
#include <cerrno>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "ppocr/image/image_from_file.hpp"


namespace
{
    struct Buffer
    {
        Buffer(std::size_t len)
            : len(len)
        {
            data = static_cast<uint8_t*>(operator new(len));
        }

        Buffer(Buffer&& other)
          : data(std::exchange(other.data, nullptr))
          , len(std::exchange(other.len, 0))
        {}

        Buffer(Buffer const&) = delete;
        Buffer& operator=(Buffer const&) = delete;

        ~Buffer()
        {
            operator delete(data);
        }

        uint8_t* data = nullptr;
        std::size_t len = 0;
    };

    struct File
    {
        File(char const* filename)
          : fd(open(filename, O_RDONLY))
        {}

        ~File()
        {
            close(fd);
        }

        int fd;
    };

    [[noreturn]]
    void throw_error(const char * filename)
    {
        std::string desc = "image_from_file: ";
        desc += filename;
        if (int errnum = errno) {
            desc += ": ";
            desc += strerror(errnum);
        }
        throw std::runtime_error(std::move(desc));
    }

    Buffer read_all(const char * filename)
    {
        File file(filename);

        struct stat st;
        if (file.fd < 0) {
            throw_error(filename);
        }

        if (int err = fstat(file.fd, &st)) {
            throw_error(filename);
        }

        if (st.st_size < 0) {
            errno = 0;
            throw_error(filename);
        }

        Buffer buf(static_cast<std::size_t>(st.st_size));
        std::size_t remaining = buf.len;
        auto* p = buf.data;
        while (ssize_t res = read(file.fd, p, remaining)) {
            if (res <= 0) {
                if (res == 0) {
                    buf.data -= remaining;
                    return 0;
                }
                throw_error(filename);
            }
            remaining -= static_cast<std::size_t>(res);
            p += res;
        }

        return buf;
    }

    struct PnmHeader
    {
        uint8_t type;
        unsigned width;
        unsigned height;
        unsigned max_value;
        uint8_t const* end_ptr;

        bool parse(uint8_t const* p, uint8_t const* end)
        {
            // pnm format:
            // 'P' ['1'-'6'] '\n'               # type
            // ('#'...'\n')*                    # comments
            // ['0'-'9']+ ' ' ['0'-'9']+ '\n'   # width height
            // ['0'-'9']+ '\n'                  # max value (when type != 1 or 4)
            // data

            if (end - p < 9) {
                return false;
            }

            type = p[1];

            // check magic ('P1'..'P6')
            if (p[0] != 'P' || (type < '1' || type > '6') || p[2] != '\n') {
                return false;
            }

            p += 3;

            // skip comments ('#'...'\n')
            while (p < end && *p == '#') {
                for (;;) {
                    ++p;
                    if (p == end) {
                        break;
                    }

                    if (*p == '\n') {
                        ++p;
                        break;
                    }
                }
            }

            auto as_chars = [](uint8_t const* p) { return reinterpret_cast<char const*>(p); };
            auto as_bytes = [](char const* p) { return reinterpret_cast<uint8_t const*>(p); };

            auto consume_uint = [&](unsigned& n, uint8_t next_ch) {
                auto res1 = std::from_chars(as_chars(p), as_chars(end), n);
                if (res1.ec != std::errc{}) {
                    return false;
                }
                p = as_bytes(res1.ptr);

                if (p == end || *p != next_ch) {
                    return false;
                }

                ++p;

                return true;
            };

            // get size (<width> ' ' <height>)
            if (!consume_uint(width, ' ') || !consume_uint(height, '\n')) {
                return false;
            }

            // max value
            max_value = 0;
            if (type != '1' && type != '4') {
                if (!consume_uint(max_value, '\n')) {
                    return false;
                }
            }

            end_ptr = p;
            return true;
        }
    };
} // anonymous namespace

namespace ppocr {

Image image_from_file(const char * filename, unsigned luminance)
{
    Buffer buf = read_all(filename);

    PnmHeader header;
    uint8_t const* end = buf.data + buf.len;
    if (!header.parse(buf.data, end)) {
        throw std::runtime_error("image_from_file: badly formed header");
    }

    constexpr char const* unsupported_max_value
      = "image_from_file: unsupported format: maximum value is 1";

    constexpr char const* invalid_data_len
      = "image_from_file: invalid data len";

    std::size_t img_size = header.width * header.height;

    if (!img_size) {
        throw std::runtime_error("image_from_file: empty image");
    }

    PtrImageData vimg(new Pixel[img_size]);
    uint8_t const* p = header.end_ptr;
    char* output = vimg.get();

    // binary data (pnm)
    if (header.type == '6') {
        if (static_cast<std::size_t>(end - p) != img_size * 3) {
            throw std::runtime_error(invalid_data_len);
        }

        for (; p < end; p += 3) {
            unsigned char c
                = ((511/*PPM_RED_WEIGHT*/   * p[0] + 511) >> 10)
                + ((396/*PPM_GREEN_WEIGHT*/ * p[1] + 511) >> 10)
                + ((117/*PPM_BLUE_WEIGHT*/  * p[2] + 511) >> 10);
            *output++ = (c < luminance) ? '-' : 'x';
        }
    }
    // binary data (pbm)
    else if (header.type == '4') {
        if (static_cast<std::size_t>(end - p) != (header.width + 7) / 8 * header.height) {
            throw std::runtime_error(invalid_data_len);
        }

        for (unsigned row = 0; row < header.height; ++row) {
            int i = 0;
            unsigned c = 0;
            for (unsigned col = 0; col < header.width; ++col) {
                if (i % 8 == 0) {
                    c = *p++;
                }
                *output++ = (c & 128) ? '-' : 'x';
                c <<= 1;
                ++i;
            }
        }
    }
    // ascii data (pnm and pbm)
    else if (header.type == '3' || header.type == '1') {
        std::size_t compunt_counter = 0;
        for (; p < end; ++p) {
            if (*p == ' ') {
                // ignored
            }
            else if (*p == '0') {
                *output++ = '-';
                if (++compunt_counter == img_size) {
                    break;
                }
            }
            else if (*p == '1') {
                *output++ = 'x';
                if (++compunt_counter == img_size) {
                    break;
                }
            }
            else if (*p == '\n') {
                // ignored
            }
            else {
                throw std::runtime_error(unsupported_max_value);
            }
        }

        // skip blank
        for (; p < end && (*p == ' ' || *p == '\n'); ++p) {
        }

        if (p != end) {
            throw std::runtime_error(invalid_data_len);
        }
    }
    else {
        throw std::runtime_error("image_from_file: unsupported format (expected 1, 3, 4 and 6)");
    }

    return Image(Bounds(header.width, header.height), std::move(vimg));
}

}
