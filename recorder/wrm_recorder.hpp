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
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Jonathan Poelen
 */

#if !defined(__WRM_RECORDER_HPP__)
#define __WRM_RECORDER_HPP__

#include <errno.h>
#include <vector>

#include "transport.hpp"
#include "RDP/RDPSerializer.hpp"
#include "meta_file.hpp"
#include "RDP/RDPDrawable.hpp"
#include "bitmap.hpp"
#include "stream.hpp"
#include "png.hpp"
#include "error.hpp"
#include "auto_buffer.hpp"
#include "cipher_transport.hpp"
#include "zlib.hpp"
#include "range_time_point.hpp"
#include "relative_time_point.hpp"
#include "capture.hpp"
#include "staticcapture.hpp"
#include "nativecapture.hpp"


template<std::size_t N>
struct HexadecimalOption
{
    unsigned char data[N];
    std::size_t size;

    HexadecimalOption()
    : size(0)
    {}

    /**
     * \param s value in hexadecimal base
     */
    bool parse(const std::string& s)
    {
        std::size_t n = s.size() / 2 + (s.size() & 1);
        if (n > N || !transform_string_hex_to_data(s, this->data))
            return false;
        this->size = n;
        while (n != N)
            this->data[n++] = 0;
        return true;
    }

private:
    static bool transform_string_hex_to_data(const std::string& s, unsigned char * pdata)
    {
        std::string::const_iterator first = s.begin();
        std::string::const_iterator last = s.end();
        char c;
        if (s.size() & 1)
            --last;
        for (; first != last; ++first, ++pdata)
        {
            if (0xf == (*pdata = transform_c_hex_to_c_data(*first)))
                return false;
            if (0xf == (c = transform_c_hex_to_c_data(*++first)))
                return false;
            *pdata = (*pdata << 4) + c;
        }
        if (s.size() & 1)
        {
            if (0xf == (*pdata = transform_c_hex_to_c_data(*first)))
                return false;
            *pdata <<= 4;
        }
        return true;
    }

    static unsigned char transform_c_hex_to_c_data(char c)
    {
        if ('a' <= c && c <= 'f')
            return c - 'a' + 0xa;
        if ('A' <= c && c <= 'F')
            return c - 'A' + 0xa;
        if ('0' > c || c > '9')
            return 0xf;
        return c - '0';
    }
};

typedef HexadecimalOption<EVP_MAX_KEY_LENGTH> HexadecimalKeyOption;
typedef HexadecimalOption<EVP_MAX_IV_LENGTH> HexadecimalIVOption;

struct InputType {
    enum enum_t {
        NOT_FOUND,
        META_TYPE,
        WRM_TYPE
    };

    typedef enum_t format_type;

    static InputType::enum_t string_to_type(const std::string& s)
    {
        if (s == "mwrm")
            return META_TYPE;
        if (s == "wrm")
            return WRM_TYPE;
        return NOT_FOUND;
    }
};


class WRMRecorder
{
    public:
    InFileTransport trans;

public:
    RDPUnserializer reader;

    Drawable * redrawable;

public:
    std::size_t idx_file;

    std::string path;
    std::size_t base_path_len;

public:
    bool only_filename;
    bool force_interpret_breakpoint;
    bool interpret_breakpoint_is_passed;

public:
    WRMRecorder(const timeval & now,
                InputType::enum_t itype,
                std::string & base_path,
                bool ignore_dir_for_meta_in_wrm,
                bool times_in_meta_are_false,
                bool force_interpret_breakpoint,
                range_time_point & range,
                std::string & in_filename,
                uint idx_start)
    : trans(0)
    , reader(&this->trans, now, 0, Rect())
    , redrawable(0)
    , idx_file(0)
    , path()
    , base_path_len(0)
    , only_filename(false)
    , force_interpret_breakpoint(false)
    , interpret_breakpoint_is_passed(false)
    {
        RDPUnserializer & reader = this->reader;
    
        this->base_path_len = base_path.length();
        this->path = base_path;
        if (this->base_path_len && this->path[this->base_path_len - 1] != '/')
        {
            this->path += '/';
            ++this->base_path_len;
        }
        
        this->only_filename = ignore_dir_for_meta_in_wrm;

        try
        {
            switch (itype) {
                case InputType::WRM_TYPE:
                {
                    const char * filename = in_filename.c_str();
                    LOG(LOG_INFO, "WRMRecorder opening file : %s", filename);
                    int fd = ::open(filename, O_RDONLY);
                    if (-1 == fd){
                        LOG(LOG_ERR, "Error opening wrm reader file : %s", strerror(errno));
                       throw Error(ERR_WRM_RECORDER_OPEN_FAILED);
                    }
                    this->trans.fd = fd;
                    if (!reader.selected_next_order())
                    {
                        std::cerr << in_filename << " is invalid wrm file" << std::endl;
                        throw Error(ERR_WRM_INVALID_FILE);
                    }
                    if (!reader.chunk_type == WRMChunk::META_FILE){
                        std::cerr << reader.data_meta << '\n'
                         << "Chunk META not found in " << filename << '\n' 
                         << ". Chunk is " << reader.chunk_type << std::endl;
                        throw Error(ERR_WRM_CHUNK_META_NOT_FOUND);
                    }
                        
                    char tmp_filename[1024];
                    size_t len = reader.stream.in_uint32_le();
                    reader.stream.in_copy_bytes((uint8_t*)tmp_filename, len);
                    tmp_filename[len] = 0;
                    --reader.remaining_order_count;
                    
                    const char * filename2 = tmp_filename;
                    if (this->only_filename)
                    {
                        const char * tmp = strrchr(filename2 + strlen(filename2), '/');
                        if (tmp){
                            filename2 = tmp+1;
                        }
                    }
                    if (this->base_path_len){
                        this->path.erase(this->base_path_len);
                        this->path += filename2;
                        filename2 = this->path.c_str();
                    }
                    
                    if (!reader.load_data(filename2)){
                        std::cerr << "invalid meta chunck in " << in_filename << std::endl;
                        throw Error(ERR_WRM_INVALID_META_CHUNK);
                    }
                    if (!reader.data_meta.files.empty())
                    {
                        if (idx_start >= reader.data_meta.files.size()){
                            std::cerr << "idx " << idx_start << " not found" << std::endl;
                            throw Error(ERR_WRM_IDX_NOT_FOUND);
                        }
                        if (!times_in_meta_are_false){
                            const std::vector<DataFile>& files = reader.data_meta.files;
                            if (files[0].start_sec){
                                const timeval tm = {files[0].start_sec, files[0].start_usec};
                                uint64_t time = 0;
                                for (uint idx = idx_start + 1; idx != files.size(); ++idx)
                                {
                                    const DataFile& data_file = files[idx];
                                    if (data_file.start_sec)
                                    {
                                        timeval tm2 = {data_file.start_sec, data_file.start_usec};
                                        uint64_t elapsed = difftimeval(tm2, tm) / 1000000;
                                        if (elapsed > range.left.time)
                                        {
                                            range.left.time -= time;
                                            break;
                                        }
                                        time = elapsed;
                                        idx_start = idx;
                                    }
                                }
                            }
                        }
                    }
                    else  if (idx_start >= reader.data_meta.files.size()){
                        std::cerr << "idx " << idx_start << " not found" << std::endl;
                        throw Error(ERR_WRM_IDX_NOT_FOUND);
                    }
                    if (idx_start != this->idx_file){
                        const char * filename = reader.data_meta.files[this->idx_file].wrm_filename.c_str();
                        ::close(this->trans.fd);
                        this->trans.fd = -1;
                        if (this->only_filename)
                        {
                            const char * tmp = strrchr(filename + strlen(filename), '/');
                            if (tmp){
                                filename = tmp+1;
                            }
                        }
                        if (this->base_path_len){
                            this->path.erase(this->base_path_len);
                            this->path += filename;
                            filename = this->path.c_str();
                        }

                        LOG(LOG_INFO, "WRMRecorder opening file : %s", filename);
                        int fd = ::open(filename, O_RDONLY);
                        if (-1 == fd){
                            LOG(LOG_ERR, "Error opening wrm reader file : %s", strerror(errno));
                           throw Error(ERR_WRM_RECORDER_OPEN_FAILED);
                        }
                        this->trans.fd = fd;

                        this->trans.total_received = 0;
                        this->trans.last_quantum_received = 0;
                        this->trans.total_sent = 0;
                        this->trans.last_quantum_sent = 0;
                        this->trans.quantum_count = 0;
                    }
                }
                break;
                case InputType::META_TYPE:
                {
                    if (!reader.load_data(in_filename.c_str()))
                    {
                        std::cerr << "open " << in_filename << ' ' << strerror(errno) << std::endl;
                        throw Error(ERR_WRM_FAILED_OPENING_META_FILENAME);
                    }
                    if (idx_start >= reader.data_meta.files.size()){
                        std::cerr << "idx " << idx_start << " not found" << std::endl;
                        throw Error(ERR_WRM_IDX_NOT_FOUND);
                    }
                    if (!times_in_meta_are_false){
                        const std::vector<DataFile>& files = reader.data_meta.files;
                        if (files[0].start_sec){
                            const timeval tm = {files[0].start_sec, files[0].start_usec};
                            uint64_t time = 0;
                            for (uint idx = idx_start + 1; idx != files.size(); ++idx)
                            {
                                const DataFile& data_file = files[idx];
                                if (data_file.start_sec)
                                {
                                    timeval tm2 = {data_file.start_sec, data_file.start_usec};
                                    uint64_t elapsed = difftimeval(tm2, tm) / 1000000;
                                    if (elapsed > range.left.time)
                                    {
                                        range.left.time -= time;
                                        break;
                                    }
                                    time = elapsed;
                                    idx_start = idx;
                                }
                            }
                        }
                    }
                    const char * filename = reader.data_meta.files[idx_start].wrm_filename.c_str();
                    
                    if (this->only_filename)
                    {
                        const char * tmp = strrchr(filename + strlen(filename), '/');
                        if (tmp){
                            filename = tmp+1;
                        }
                    }
                    if (this->base_path_len){
                        this->path.erase(this->base_path_len);
                        this->path += filename;
                        filename = this->path.c_str();
                    }
                    

                    LOG(LOG_INFO, "WRMRecorder opening file : %s", filename);
                    int fd = ::open(filename, O_RDONLY);
                    if (-1 == fd){
                        LOG(LOG_ERR, "Error opening wrm reader file : %s", strerror(errno));
                       throw Error(ERR_WRM_RECORDER_OPEN_FAILED);
                    }
                    this->trans.fd = fd;
                    if (reader.selected_next_order() 
                    && reader.chunk_type == WRMChunk::META_FILE){
                        reader.stream.p = reader.stream.end;
                        reader.remaining_order_count = 0;
                    }
                    if (!reader.chunk_type == WRMChunk::META_FILE){
                        std::cerr << reader.data_meta << '\n'
                         << "Chunk META not found in " << filename << '\n' 
                         << ". Chunk is " << reader.chunk_type << std::endl;
                        throw Error(ERR_WRM_CHUNK_META_NOT_FOUND);
                    }
                }
                break;
                default:
                    std::cerr << "Input type not found" << std::endl;
                    throw Error(ERR_WRM);
            }
            this->idx_file = idx_start + 1;
            this->force_interpret_breakpoint = force_interpret_breakpoint;
        }
        catch (const Error& error)
        {
            std::cerr << "Error " << error.id << ": " << strerror(error.errnum) << std::endl;
            throw error;
        }
    }

    void load_png_context(Drawable& drawable)
    {
        if (this->idx_file > 1 
        && this->reader.data_meta.files.size() >= this->idx_file
        && !this->reader.data_meta.files[this->idx_file - 1].png_filename.empty())
        {
            this->redrawable = &drawable;
            this->load_context(this->reader.data_meta.files[this->idx_file - 1].png_filename.c_str());
            this->redrawable = 0;
        }
    }

    ~WRMRecorder()
    {
        ::close(this->trans.fd);
    }

public:
    void load_context(const char * filename)
    {
        if (this->redrawable)
        {
            if (this->only_filename)
            {
                const char * tmp = strrchr(filename + strlen(filename), '/');
                if (tmp){
                    filename = tmp+1;
                }
            }
            if (this->base_path_len){
                this->path.erase(this->base_path_len);
                this->path += filename;
                filename = this->path.c_str();
            }
            std::FILE* fd = std::fopen(filename, "r");
            if (0 == fd)
            {
                LOG(LOG_ERR, "open context screen %s: %s", filename, strerror(errno));
                throw Error(ERR_RECORDER_FAILED_TO_OPEN_TARGET_FILE, errno);
            }

            read_png24(fd,
                       this->redrawable->data,
                       this->redrawable->width,
                       this->redrawable->height,
                       this->redrawable->rowsize);
            fclose(fd);
        }
    }


private:
    void recv_rect(Rect& rect)
    {
        rect.x = this->reader.stream.in_uint16_le();
        rect.y = this->reader.stream.in_uint16_le();
        rect.cx = this->reader.stream.in_uint16_le();
        rect.cy = this->reader.stream.in_uint16_le();
    }

    void recv_brush(RDPBrush& brush)
    {
        brush.org_x = this->reader.stream.in_uint8();
        brush.org_y = this->reader.stream.in_uint8();
        brush.style = this->reader.stream.in_uint8();
        brush.hatch = this->reader.stream.in_uint8();
        brush.extra[0] = this->reader.stream.in_uint8();
        brush.extra[1] = this->reader.stream.in_uint8();
        brush.extra[2] = this->reader.stream.in_uint8();
        brush.extra[3] = this->reader.stream.in_uint8();
        brush.extra[4] = this->reader.stream.in_uint8();
        brush.extra[5] = this->reader.stream.in_uint8();
        brush.extra[6] = this->reader.stream.in_uint8();
    }

    void recv_pen(RDPPen& pen)
    {
        pen.color = this->reader.stream.in_uint32_le();
        pen.style = this->reader.stream.in_uint8();
        pen.width = this->reader.stream.in_uint8();
    }

public:
    timeval get_start_time_order()
    {
        timeval time;
        time.tv_sec = this->reader.stream.in_uint64_be();
        time.tv_usec = this->reader.stream.in_uint64_be();
        return time;
    }

    void interpret_order()
    {
        switch (this->reader.chunk_type)
        {
            case WRMChunk::TIME_START:
            {
                this->reader.stream.p = this->reader.stream.end;
                this->reader.remaining_order_count = 0;
            }
            break;
            case WRMChunk::META_FILE:
            {
                this->reader.stream.p = this->reader.stream.end;
                this->reader.remaining_order_count = 0;
            }
            break;
            case WRMChunk::NEXT_FILE_ID:
            {
                this->idx_file = this->reader.stream.in_uint32_le();
                if (this->reader.data_meta.files.size() <= this->idx_file)
                {
                    LOG(LOG_ERR, "WRMRecorder : idx(%d) not found in meta", (int)this->idx_file);
                    throw Error(ERR_RECORDER_META_REFERENCE_WRM);
                }
                const char * filename = this->reader.data_meta.files[this->idx_file].wrm_filename.c_str();
                ::close(this->trans.fd);
                this->trans.fd = -1;
                if (this->only_filename)
                {
                    const char * tmp = strrchr(filename + strlen(filename), '/');
                    if (tmp){
                        filename = tmp+1;
                    }
                }
                if (this->base_path_len){
                    this->path.erase(this->base_path_len);
                    this->path += filename;
                    filename = this->path.c_str();
                }

                LOG(LOG_INFO, "WRMRecorder opening file : %s", filename);
                int fd = ::open(filename, O_RDONLY);
                if (-1 == fd){
                    LOG(LOG_ERR, "Error opening wrm reader file : %s", strerror(errno));
                   throw Error(ERR_WRM_RECORDER_OPEN_FAILED);
                }
                this->trans.fd = fd;

                this->trans.total_received = 0;
                this->trans.last_quantum_received = 0;
                this->trans.total_sent = 0;
                this->trans.last_quantum_sent = 0;
                this->trans.quantum_count = 0;
                --this->reader.remaining_order_count;
                this->load_context(this->reader.data_meta.files[this->idx_file].png_filename.c_str());
            }
            break;
            case WRMChunk::BREAKPOINT:
            {
                if (!this->interpret_breakpoint_is_passed || this->force_interpret_breakpoint){
                    /*uint16_t width = */this->reader.stream.in_uint16_le();
                    /*uint16_t height = */this->reader.stream.in_uint16_le();
                    /*uint8_t bpp = */this->reader.stream.in_uint8();
                    this->reader.timer_cap.tv_sec = this->reader.stream.in_uint64_le();
                    this->reader.timer_cap.tv_usec = this->reader.stream.in_uint64_le();
                    --this->reader.remaining_order_count;

                    this->reader.selected_next_order();

                    this->reader.common.order = this->reader.stream.in_uint8();
                    this->recv_rect(this->reader.common.clip);
                    //this->reader.common.str(texttest, 10000);
                    //std::cout << "interpret_order: " << texttest << '\n';

                    this->reader.opaquerect.color = this->reader.stream.in_uint32_le();
                    this->recv_rect(this->reader.opaquerect.rect);
                    //std::cout << "interpret_order: ";
                    //this->reader.opaquerect.print(Rect(0,0,0,0));

                    this->reader.destblt.rop = this->reader.stream.in_uint8();
                    this->recv_rect(this->reader.destblt.rect);
                    //std::cout << "interpret_order: ";
                    //this->reader.destblt.print(Rect(0,0,0,0));

                    this->reader.patblt.rop = this->reader.stream.in_uint8();
                    this->reader.patblt.back_color = this->reader.stream.in_uint32_le();
                    this->reader.patblt.fore_color = this->reader.stream.in_uint32_le();
                    this->recv_brush(this->reader.patblt.brush);
                    this->recv_rect(this->reader.patblt.rect);
                    //std::cout << "interpret_order: ";
                    //this->reader.patblt.print(Rect(0,0,0,0));

                    this->reader.scrblt.rop = this->reader.stream.in_uint8();
                    this->reader.scrblt.srcx = this->reader.stream.in_uint16_le();
                    this->reader.scrblt.srcy = this->reader.stream.in_uint16_le();
                    this->recv_rect(this->reader.scrblt.rect);
                    //std::cout << "interpret_order: ";
                    //this->reader.scrblt.print(Rect(0,0,0,0));

                    this->reader.memblt.rop = this->reader.stream.in_uint8();
                    this->reader.memblt.srcx = this->reader.stream.in_uint16_le();
                    this->reader.memblt.srcy = this->reader.stream.in_uint16_le();
                    this->reader.memblt.cache_id = this->reader.stream.in_uint16_le();
                    this->reader.memblt.cache_idx = this->reader.stream.in_uint16_le();
                    this->recv_rect(this->reader.memblt.rect);
                    //std::cout << "interpret_order: ";
                    //this->reader.memblt.print(Rect(0,0,0,0));

                    this->reader.lineto.rop2 = this->reader.stream.in_uint8();
                    this->reader.lineto.startx = this->reader.stream.in_uint16_le();
                    this->reader.lineto.starty = this->reader.stream.in_uint16_le();
                    this->reader.lineto.endx = this->reader.stream.in_uint16_le();
                    this->reader.lineto.endy = this->reader.stream.in_uint16_le();
                    this->reader.lineto.back_mode = this->reader.stream.in_uint8();
                    this->reader.lineto.back_color = this->reader.stream.in_uint32_le();
                    this->recv_pen(this->reader.lineto.pen);
                    //std::cout << "interpret_order: ";
                    //this->reader.lineto.print(Rect(0,0,0,0));

                    this->reader.glyphindex.back_color = this->reader.stream.in_uint32_le();
                    this->reader.glyphindex.fore_color = this->reader.stream.in_uint32_le();
                    this->reader.glyphindex.f_op_redundant = this->reader.stream.in_uint16_le();
                    this->reader.glyphindex.fl_accel = this->reader.stream.in_uint16_le();
                    this->reader.glyphindex.glyph_x = this->reader.stream.in_uint16_le();
                    this->reader.glyphindex.glyph_y = this->reader.stream.in_uint16_le();
                    this->reader.glyphindex.ui_charinc = this->reader.stream.in_uint16_le();
                    this->reader.glyphindex.cache_id = this->reader.stream.in_uint8();
                    this->reader.glyphindex.data_len = this->reader.stream.in_uint8();
                    this->recv_rect(this->reader.glyphindex.bk);
                    this->recv_rect(this->reader.glyphindex.op);
                    this->recv_brush(this->reader.glyphindex.brush);
                    this->reader.glyphindex.data = (uint8_t*)malloc(this->reader.glyphindex.data_len);
                    this->reader.stream.in_copy_bytes(this->reader.glyphindex.data, this->reader.glyphindex.data_len);
                    //std::cout << "interpret_order: ";
                    //this->reader.glyphindex.print(Rect(0,0,0,0));

                    this->reader.order_count = this->reader.stream.in_uint16_le();
                    //std::cout << "\ninterpret_order: "  << this->reader.order_count << '\n';

                    this->reader.bmp_cache.small_entries = this->reader.stream.in_uint16_le();
                    this->reader.bmp_cache.small_size = this->reader.stream.in_uint16_le();
                    this->reader.bmp_cache.medium_entries = this->reader.stream.in_uint16_le();
                    this->reader.bmp_cache.medium_size = this->reader.stream.in_uint16_le();
                    this->reader.bmp_cache.big_entries = this->reader.stream.in_uint16_le();
                    this->reader.bmp_cache.big_size = this->reader.stream.in_uint16_le();
                    uint32_t stamp = this->reader.stream.in_uint32_le();

                    this->reader.bmp_cache.reset();
                    this->reader.bmp_cache.stamp = stamp;
                    this->reader.remaining_order_count = 0;

                    z_stream zstrm;
                    zstrm.zalloc = 0;
                    zstrm.zfree = 0;
                    zstrm.opaque = 0;
                    int ret;
                    const int Bpp = 3;
                    uint8_t * buffer = NULL;
                    while (1){
                        BStream stream(14);
                        this->reader.trans->recv(&stream.end, 14);
                        uint16_t idx = stream.in_uint16_le();
                        uint32_t stamp = stream.in_uint32_le();
                        uint16_t cx = stream.in_uint16_le();
                        uint16_t cy = stream.in_uint16_le();
                        uint32_t buffer_size = stream.in_uint32_le();
                        if (idx == 8192 * 3 + 1){
                            break;
                        }

                        BStream image_stream(buffer_size);
                        this->reader.trans->recv(&image_stream.end, buffer_size);

                        zstrm.avail_in = buffer_size;
                        zstrm.next_in = image_stream.data;

                        buffer = (uint8_t*)malloc(cx * cy * Bpp);
                        zstrm.avail_out = cx * cy * Bpp;
                        zstrm.next_out = buffer;

                        if ((ret = inflateInit(&zstrm)) != Z_OK)
                        {
                            LOG(LOG_ERR, "zlib: inflateInit: %d", ret);
                            throw Error(ERR_WRM_RECORDER_ZIP_UNCOMPRESS);
                        }

                        ret = inflate(&zstrm, Z_FINISH);
                        inflateEnd(&zstrm);

                        if (ret != Z_STREAM_END)
                        {
                            LOG(LOG_ERR, "zlib: inflate: %d", ret);
                            throw Error(ERR_WRM_RECORDER_ZIP_UNCOMPRESS);
                        }

                        uint cid = idx / 8192;
                        uint cidx = idx % 8192;
                        this->reader.bmp_cache.stamps[cid][cidx] = stamp;
                        if (this->reader.bmp_cache.cache[cid][cidx] != 0){
                            LOG(LOG_ERR, "bmp_cache already used at %u:%u", cid, cidx);
                        }
                        this->reader.bmp_cache.cache[cid][cidx] = new Bitmap(24, 0, cx, cy, buffer, cx*cy);
                    }
                    this->interpret_breakpoint_is_passed = true;

                }
                else {
                    this->reader.stream.p = this->reader.stream.end;
                    this->reader.remaining_order_count = 0;

                    this->reader.selected_next_order();
                    this->reader.remaining_order_count = 0;
                    while (1){
                        this->reader.stream.init(14);
                        this->reader.trans->recv(&this->reader.stream.end, 14);
                        uint16_t idx = this->reader.stream.in_uint16_le();
                        this->reader.stream.p += 8;
                        uint32_t buffer_size = this->reader.stream.in_uint32_le();
                        if (idx == 8192 * 3 + 1){
                            break;
                        }
                        this->reader.stream.init(buffer_size);
                        this->reader.trans->recv(&this->reader.stream.end, buffer_size);
                    }
                }
            }
            break;
            default:
                this->reader.interpret_order();
                break;
        }
    }

    bool next_order()
    {
        if (this->reader.selected_next_order()){
            this->interpret_order();
            return true;
        }
        return false;
    }
    
    void to_png(const char* outfile,
                std::size_t start, std::size_t stop, std::size_t interval,
                unsigned resize_width, unsigned resize_height,
                uint frame_limit,
                bool screenshot_start, bool no_screenshot_stop,
                bool screenshot_all)
    {
        printf("to png -> %s width=%u height=%u\n", outfile, this->reader.data_meta.width, this->reader.data_meta.height);

        StaticCapture capture(this->reader.data_meta.width, this->reader.data_meta.height, outfile, true);

        this->reader.consumer = &capture;
        this->load_png_context(capture.drawable);
        const uint64_t coeff_sec_to_usec = 1000000;
        uint64_t timercompute_microsec = 0;
        uint64_t timercompute_chunk_time_value = 0;
        if (start > 0){
            uint64_t msec = coeff_sec_to_usec * start;
            if (timercompute_microsec < msec){
                while (this->reader.selected_next_order())
                {
                    if (this->reader.chunk_type == WRMChunk::TIMESTAMP && timercompute_microsec < msec){
                        timercompute_chunk_time_value = this->reader.stream.in_uint64_be();
                        timercompute_microsec += timercompute_chunk_time_value;
                        --this->reader.remaining_order_count;    
                        break;
                    }
                    this->interpret_order();
                }
            }
            if (timercompute_microsec == 0){
                return /*0*/;
            }
            timercompute_microsec = 0;
        }
        if (1 && screenshot_start){
            capture.dump_png();
        }

        uint frame = 0;
        uint64_t mtime = coeff_sec_to_usec * interval;
        uint64_t msecond = coeff_sec_to_usec * (stop - start);
        uint64_t minterval = 0;

        while (this->reader.selected_next_order())
        {
            if (this->reader.chunk_type == WRMChunk::TIMESTAMP) {
                timercompute_chunk_time_value = this->reader.stream.in_uint64_be();
                timercompute_microsec += timercompute_chunk_time_value;
                --this->reader.remaining_order_count;
                uint64_t usec = timercompute_microsec;
                if (usec >= mtime)
                {
                    capture.dump_png();
                    timercompute_microsec = 0;
                    if (++frame == frame_limit){
                        break;
                    }
                    if (screenshot_all){
                        minterval += usec - mtime;
                        while (minterval >= mtime){
                            capture.dump_png();
                            minterval -= mtime;
                        }
                    }
                }

                if (msecond <= usec){
                    msecond = 0;
                    break;
                }
                else{
                    msecond -= usec;
                }
            }
            else{
                this->interpret_order();
            }
        }
        if (!no_screenshot_stop && msecond && frame != frame_limit){
            capture.dump_png();
            //++frame;
        }
        LOG(LOG_INFO, "to png -> %s done", outfile);
        //return frame;
    }

    void to_png_list(const char* outfile,
                const std::vector<relative_time_point>& capture_points,
                unsigned resize_width, unsigned resize_height,
                bool no_screenshot_stop)
    {
        if (capture_points.empty()){
            return ;
        }

        StaticCapture capture(this->reader.data_meta.width, this->reader.data_meta.height, outfile, true);
        this->reader.consumer = &capture;
        this->load_png_context(capture.drawable);

        typedef std::vector<relative_time_point>::const_iterator iterator;
        iterator it = capture_points.begin();

        const uint64_t coeff_sec_to_usec = 1000000;
        uint64_t timercompute_microsec = 0;
        uint64_t timercompute_chunk_time_value = 0;
        uint64_t start = it->point.time;
        if (start){
            uint64_t msec = coeff_sec_to_usec * start;
            uint64_t tmp = 0;
            if (msec > 0){
                tmp = timercompute_microsec;
                if (timercompute_microsec < msec){
                    while (this->reader.selected_next_order())
                    {
                        if (this->reader.chunk_type == WRMChunk::TIMESTAMP && timercompute_microsec < msec){
                            timercompute_chunk_time_value = this->reader.stream.in_uint64_be();
                            timercompute_microsec += timercompute_chunk_time_value;
                            --this->reader.remaining_order_count;    
                            tmp = timercompute_microsec;
                            break;
                        }
                        this->interpret_order();
                    }
                }
            }
            timercompute_microsec = 0;
            if (tmp == 0){
                return /*0*/;
            }
        }

        capture.dump_png();
        iterator end = capture_points.end();
        if (++it == end)
            return;
        uint64_t mtime = 0;

        while (this->reader.selected_next_order())
        {
            if (this->reader.chunk_type == WRMChunk::TIMESTAMP) {
                timercompute_chunk_time_value = this->reader.stream.in_uint64_be();
                timercompute_microsec += timercompute_chunk_time_value;
                --this->reader.remaining_order_count;
                mtime += timercompute_microsec;
                while (mtime >= coeff_sec_to_usec * it->point.time)
                {
                    capture.dump_png();
                    if (++it == end)
                        return;
                }
                timercompute_microsec = 0;
            }
            else
            {
                this->interpret_order();
            }
        }
        if (!no_screenshot_stop){
            capture.dump_png();
        }
    }
    
    void to_one_wrm(const char* outfile,
                    std::size_t start, std::size_t stop, const char* metaname,
                    CipherMode::enum_t mode,
                    const unsigned char * key, const unsigned char * iv)
    {
        LOG(LOG_INFO, "to one wrm");
        timeval now;
        gettimeofday(&now, NULL);

        NativeCapture capture(now,
                              this->reader.data_meta.width,
                              this->reader.data_meta.height,
                              outfile, metaname,
                              mode, key, iv
                             );
                             
        this->reader.consumer = &capture;
        uint64_t timercompute_microsec = 0;
        uint64_t timercompute_chunk_time_value = 0;

        timeval mstart = {0,0};
        while (this->reader.selected_next_order())
        {
            if (this->reader.chunk_type == WRMChunk::TIME_START){
                mstart = this->get_start_time_order();
                break;
            }
            if (this->reader.chunk_type == WRMChunk::TIMESTAMP){
                timercompute_chunk_time_value = this->reader.stream.in_uint64_be();
                timercompute_microsec += timercompute_chunk_time_value;
                --this->reader.remaining_order_count;
                mstart.tv_sec = 0;
                mstart.tv_usec = 0;
                break;
            }
            this->interpret_order();
        }

        const uint64_t coeff_sec_to_usec = 1000000;
        uint64_t msec = coeff_sec_to_usec * start;
        uint64_t mtime = 0;
        if (msec > 0){
            mtime = timercompute_microsec;
            if (timercompute_microsec < msec){
                while (this->reader.selected_next_order())
                {
                    if (this->reader.chunk_type == WRMChunk::TIMESTAMP && timercompute_microsec < msec){
                        timercompute_chunk_time_value = this->reader.stream.in_uint64_be();
                        timercompute_microsec += timercompute_chunk_time_value;
                        --this->reader.remaining_order_count;    
                        mtime = timercompute_microsec;
                        break;
                    }
                    this->interpret_order();
                }
            }
        }
        timercompute_microsec = 0;

        if (start && !mtime){
            return /*0*/;
        }

        if (mstart.tv_sec != 0){
            if (mtime){
                uint64_t tmp_usec = mstart.tv_usec + mtime;
                mstart.tv_sec += (tmp_usec / 1000000);
                mstart.tv_usec = (tmp_usec % 1000000);
            }
            capture.send_time_start(mstart);
        }
        else {
            capture.write_start_in_meta(mstart);
        }

        if (mtime){
            capture.recorder.timestamp(mtime);
            mtime += capture.recorder.timer.tv_usec;
            capture.recorder.timer.tv_sec += mtime / 1000000;
            capture.recorder.timer.tv_usec = mtime % 1000000;
        }

        timercompute_microsec = mtime - start;
        mtime = coeff_sec_to_usec * (stop - start);
        BStream& stream = this->reader.stream;

        while (this->reader.selected_next_order())
        {
            if (this->reader.chunk_type == WRMChunk::TIMESTAMP) {
                timercompute_chunk_time_value = this->reader.stream.in_uint64_be();
                timercompute_microsec += timercompute_chunk_time_value;
                --this->reader.remaining_order_count;
                if (timercompute_chunk_time_value) {
                    capture.recorder.timestamp(timercompute_chunk_time_value);
                    timercompute_chunk_time_value += capture.recorder.timer.tv_usec;
                    capture.recorder.timer.tv_sec += timercompute_chunk_time_value / 1000000;
                    capture.recorder.timer.tv_usec = timercompute_chunk_time_value % 1000000;
                }

                if (mtime <= timercompute_microsec){
                    break;
                }
            }
            else {
                switch (this->reader.chunk_type) {
                    case WRMChunk::NEXT_FILE_ID:
                        this->interpret_order();
                        break;
                    case WRMChunk::META_FILE:
                    case WRMChunk::TIME_START:
                        this->reader.stream.p = this->reader.stream.end;
                        this->reader.remaining_order_count = 0;
                        break;
                    case WRMChunk::BREAKPOINT:
                    {
                        this->reader.stream.p = this->reader.stream.end;
                        this->reader.remaining_order_count = 0;
                        this->reader.selected_next_order();

                        while (1)
                        {
                            this->reader.stream.init(14);
                            this->reader.trans->recv(&this->reader.stream.end, 14);
                            if (this->reader.stream.in_uint16_le() == 8192 * 3 + 1){
                                break;
                            }
                            this->reader.stream.p += 8;
                            uint32_t buffer_size = this->reader.stream.in_uint32_le();
                            this->reader.stream.init(buffer_size);
                            this->reader.trans->recv(&this->reader.stream.end, buffer_size);
                        }

                        this->reader.stream.p = this->reader.stream.end;
                        this->reader.remaining_order_count = 0;
                    }
                        break;
                    default:
                        capture.recorder.trans->send(stream.data, stream.size());
                        this->reader.stream.p = this->reader.stream.end;
                        this->reader.remaining_order_count = 0;
                        break;
                }
            }
        }
    } 
    
    
    void to_wrm(const char* outfile,
                std::size_t start, std::size_t stop, std::size_t interval,
                uint frame_limit,
                bool screenshot_start, bool screenshot_wrm,
                const char* metaname,
                CipherMode::enum_t mode,
                const unsigned char * key, const unsigned char * iv
    )
    {
        struct timeval now;
        gettimeofday(&now, NULL);
        Capture capture(now, this->reader.data_meta.width,
                        this->reader.data_meta.height,
                        outfile, metaname,
                        0, 0, true,
                        mode, key, iv);
        this->reader.consumer = &capture;
        uint64_t timercompute_microsec = 0;
        uint64_t timercompute_chunk_time_value = 0;
        
        timeval mstart = {0,0};
        while (this->reader.selected_next_order())
        {
            if (this->reader.chunk_type == WRMChunk::TIME_START){
                mstart = this->get_start_time_order();
                break;
            }
            if (this->reader.chunk_type == WRMChunk::TIMESTAMP)
            {
                timercompute_chunk_time_value = this->reader.stream.in_uint64_be();
                timercompute_microsec += timercompute_chunk_time_value;
                --this->reader.remaining_order_count;
                mstart.tv_sec = 0;
                mstart.tv_usec = 0;
                break;
            }
            this->interpret_order();
        }

        static const uint64_t coeff_sec_to_usec = 1000000;
        uint64_t msec = coeff_sec_to_usec * start;
        uint64_t tmp = 0;
        if (msec > 0){
            tmp = timercompute_microsec;
            if (timercompute_microsec < msec){
                while (this->reader.selected_next_order())
                {
                    if (this->reader.chunk_type == WRMChunk::TIMESTAMP && timercompute_microsec < msec){
                        timercompute_chunk_time_value = this->reader.stream.in_uint64_be();
                        timercompute_microsec += timercompute_chunk_time_value;
                        --this->reader.remaining_order_count;    
                        tmp = timercompute_microsec;
                        break;
                    }
                    this->interpret_order();
                }
            }
        }
        timercompute_microsec = 0;
        uint64_t mtime = tmp;

        if (start && !mtime){
            return /*0*/;
        }

        if (mstart.tv_sec != 0)
        {
            if (mtime){
                uint64_t tmp_usec = mstart.tv_usec + mtime;
                mstart.tv_sec += (tmp_usec / 1000000);
                mstart.tv_usec = (tmp_usec % 1000000);
            }
            capture.start(mstart);
        }
        else {
            capture.start_with_invalid_now();
        }

        if (mtime){
            capture.timestamp(mtime);
            timeval & timer = capture.timer();
            mtime += timer.tv_usec;
            timer.tv_sec += mtime / 1000000;
            timer.tv_usec = mtime % 1000000;
        }

        if (screenshot_wrm && screenshot_start)
            capture.dump_png();

        //uint64_t chunk_time = 0;
        timercompute_microsec = mtime - start;
        uint frame = 0;
        uint64_t msecond = coeff_sec_to_usec * (stop - start);
        mtime = coeff_sec_to_usec * interval;

        while (this->reader.selected_next_order())
        {
            if (this->reader.chunk_type == WRMChunk::TIMESTAMP) {
                timercompute_chunk_time_value = this->reader.stream.in_uint64_be();
                timercompute_microsec += timercompute_chunk_time_value;
                --this->reader.remaining_order_count;

                uint64_t usec = timercompute_microsec;
                if (timercompute_chunk_time_value) {
                    //chunk_time += timercompute_chunk_time_value;
                    //std::cout << "chunk_time: " << chunk_time << '\n';
                    capture.timestamp(timercompute_chunk_time_value);
                    timeval now;
                    gettimeofday(&now, NULL);
                    timeval & timer = capture.timer();
                    timercompute_chunk_time_value += timer.tv_usec;
                    timer.tv_sec += timercompute_chunk_time_value / 1000000;
                    timer.tv_usec = timercompute_chunk_time_value % 1000000;
                }

                if (usec >= mtime) {
                    /*if (chunk_time) {
                        std::cout << "timestamp + breakpoint chunk_time: " <<   chunk_time  << '\n';
                        capture.timestamp(chunk_time);
                        chunk_time = 0;
                    }*/
                    capture.breakpoint(capture.timer());
                    if (screenshot_wrm)
                        capture.dump_png();
                    timercompute_microsec = 0;
                    if (++frame == frame_limit)
                        break;
                }

                if (msecond <= usec){
                    msecond = 0;
                    break;
                } else {
                    msecond -= usec;
                }
            }
            else {
                /*if (chunk_time) {
                     std::cout << "timestamp chunk_time: " << chunk_time  << '\n';
                     capture.timestamp(chunk_time);
                     chunk_time = 0;
                }*/

                this->interpret_order();
            }
        }

        /*if (chunk_time) {
             capture.timestamp(chunk_time);
        }*/

        //return frame;
    }
    
       
};

#endif
