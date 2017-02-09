/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2017
   Author(s): Christophe Grosjean, Jonatan Poelen
*/

#pragma once


#include "utils/log.hpp"

#include "utils/difftimeval.hpp"

#include "gdi/capture_api.hpp"

#include "core/RDP/RDPDrawable.hpp"

#include "capture/video_recorder.hpp"
#include "capture/flv_params.hpp"

#include <cerrno>
#include <cstddef>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>


struct videocapture_FilenameGenerator_PATH_FILE_EXTENSION
{
private:
    char         path[1024];
    char         filename[1012];
    char         extension[12];
    unsigned     pid;
    mutable char filename_gen[1024];

    const char * last_filename;
    unsigned     last_num;

public:
    videocapture_FilenameGenerator_PATH_FILE_EXTENSION(
        const char * const prefix,
        const char * const filename,
        const char * const extension)
    : pid(getpid())
    , last_filename(nullptr)
    , last_num(-1u)
    {
        if (strlen(prefix) > sizeof(this->path) - 1
         || strlen(filename) > sizeof(this->filename) - 1
         || strlen(extension) > sizeof(this->extension) - 1) {
            throw Error(ERR_TRANSPORT);
        }

        strcpy(this->path, prefix);
        strcpy(this->filename, filename);
        strcpy(this->extension, extension);

        this->filename_gen[0] = 0;
    }

    const char * get(unsigned count) const
    {
        if (count == this->last_num && this->last_filename) {
            return this->last_filename;
        }

        using std::snprintf;
        snprintf( this->filename_gen, sizeof(this->filename_gen), "%s%s%s", this->path
                , this->filename, this->extension);
        return this->filename_gen;
    }

    void set_last_filename(unsigned num, const char * name)
    {
        this->last_num = num;
        this->last_filename = name;
    }

private:
    videocapture_FilenameGenerator_PATH_FILE_EXTENSION(videocapture_FilenameGenerator_PATH_FILE_EXTENSION const &) = delete;
    videocapture_FilenameGenerator_PATH_FILE_EXTENSION& operator=(videocapture_FilenameGenerator_PATH_FILE_EXTENSION const &) = delete;
};


struct videocapture_FilenameGenerator_PATH_FILE_COUNT_EXTENSION
{
private:
    char         path[1024];
    char         filename[1012];
    char         extension[12];
    unsigned     pid;
    mutable char filename_gen[1024];

    const char * last_filename;
    unsigned     last_num;

public:
    videocapture_FilenameGenerator_PATH_FILE_COUNT_EXTENSION(
        const char * const prefix,
        const char * const filename,
        const char * const extension)
    : pid(getpid())
    , last_filename(nullptr)
    , last_num(-1u)
    {
        if (strlen(prefix) > sizeof(this->path) - 1
         || strlen(filename) > sizeof(this->filename) - 1
         || strlen(extension) > sizeof(this->extension) - 1) {
            throw Error(ERR_TRANSPORT);
        }

        strcpy(this->path, prefix);
        strcpy(this->filename, filename);
        strcpy(this->extension, extension);

        this->filename_gen[0] = 0;
    }

    const char * get(unsigned count) const
    {
        if (count == this->last_num && this->last_filename) {
            return this->last_filename;
        }

        using std::snprintf;
        snprintf( this->filename_gen, sizeof(this->filename_gen), "%s%s-%06u%s", this->path
                , this->filename, count, this->extension);
        return this->filename_gen;
    }

    void set_last_filename(unsigned num, const char * name)
    {
        this->last_num = num;
        this->last_filename = name;
    }

private:
    videocapture_FilenameGenerator_PATH_FILE_COUNT_EXTENSION(videocapture_FilenameGenerator_PATH_FILE_COUNT_EXTENSION const &) = delete;
    videocapture_FilenameGenerator_PATH_FILE_COUNT_EXTENSION& operator=(videocapture_FilenameGenerator_PATH_FILE_COUNT_EXTENSION const &) = delete;
};




struct videocapture_OutFilenameSequenceSeekableTransport_COUNT : public Transport
{
    char buf_current_filename_[1024];
    videocapture_FilenameGenerator_PATH_FILE_COUNT_EXTENSION buf_filegen_;
    int buf_buf_fd;
    unsigned buf_num_file_;
    int buf_groupid_;

public:
    videocapture_OutFilenameSequenceSeekableTransport_COUNT(
        const char * const prefix,
        const char * const filename,
        const char * const extension,
        const int groupid,
        auth_api * authentifier = nullptr)
    : buf_filegen_(prefix, filename, extension)
    , buf_buf_fd(-1)
    , buf_num_file_(0)
    , buf_groupid_(groupid)
    {
            this->buf_current_filename_[0] = 0;
            if (authentifier) {
                this->set_authentifier(authentifier);
            }
    }

    const videocapture_FilenameGenerator_PATH_FILE_COUNT_EXTENSION * seqgen() const noexcept
    { return &this->buf_filegen_; }

    void seek(int64_t offset, int whence) override {
        if (static_cast<off64_t>(-1) == lseek64(this->buf_buf_fd, offset, whence)){
            throw Error(ERR_TRANSPORT_SEEK_FAILED, errno);
        }
    }

    bool next() override {
        if (this->status == false) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
        ssize_t tmpres = 1;
        if (this->buf_buf_fd != -1) {
            ::close(this->buf_buf_fd);
            this->buf_buf_fd = -1;
            // LOG(LOG_INFO, "\"%s\" -> \"%s\".", this->current_filename, this->rename_to);
            
            this->buf_filegen_.set_last_filename(-1u, "");
            const char * filename = this->buf_filegen_.get(this->buf_num_file_);
            this->buf_filegen_.set_last_filename(this->buf_num_file_, this->buf_current_filename_);
            
            const int res = ::rename(this->buf_current_filename_, filename);
            if (res < 0) {
                LOG( LOG_ERR, "renaming file \"%s\" -> \"%s\" failed erro=%u : %s\n"
                   , this->buf_current_filename_, filename, errno, strerror(errno));
                tmpres = 1;
            }
            else {
                this->buf_current_filename_[0] = 0;
                ++this->buf_num_file_;
                this->buf_filegen_.set_last_filename(-1u, "");
                tmpres = 0;
            }
        }
        const ssize_t res = tmpres;
        if (res) {
            this->status = false;
            if (res < 0){
                LOG(LOG_ERR, "Write to transport failed (M): code=%d", errno);
                throw Error(ERR_TRANSPORT_WRITE_FAILED, -res);
            }
            throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
        }
        ++this->seqno;
        return true;
    }

    bool disconnect() override {
        ssize_t tmpres = 1;
        if (this->buf_buf_fd != -1) {
            ::close(this->buf_buf_fd);
            this->buf_buf_fd = -1;
            // LOG(LOG_INFO, "\"%s\" -> \"%s\".", this->current_filename, this->rename_to);
            
            this->buf_filegen_.set_last_filename(-1u, "");
            const char * filename = this->buf_filegen_.get(this->buf_num_file_);
            this->buf_filegen_.set_last_filename(this->buf_num_file_, this->buf_current_filename_);
            
            const int res = ::rename(this->buf_current_filename_, filename);
            if (res < 0) {
                LOG( LOG_ERR, "renaming file \"%s\" -> \"%s\" failed erro=%u : %s\n"
                   , this->buf_current_filename_, filename, errno, strerror(errno));
                tmpres = 1;
            }
            else {
                this->buf_current_filename_[0] = 0;
                ++this->buf_num_file_;
                this->buf_filegen_.set_last_filename(-1u, "");
                
                tmpres = 0;
            }
        }
        return !tmpres;
    }

    void request_full_cleaning() override {
        unsigned i = this->buf_num_file_ + 1;
        while (i > 0 && !::unlink(this->buf_filegen_.get(--i))) {
        }
        if (-1 != this->buf_buf_fd) {
            ::close(this->buf_buf_fd);
            this->buf_buf_fd = -1;
        }
    }

    ~videocapture_OutFilenameSequenceSeekableTransport_COUNT() {
        if (this->buf_buf_fd != -1) {
            ::close(this->buf_buf_fd);
            this->buf_buf_fd = -1;
            // LOG(LOG_INFO, "\"%s\" -> \"%s\".", this->current_filename, this->rename_to);
            
            this->buf_filegen_.set_last_filename(-1u, "");
            const char * filename = this->buf_filegen_.get(this->buf_num_file_);
            this->buf_filegen_.set_last_filename(this->buf_num_file_, this->buf_current_filename_);
            
            const int res = ::rename(this->buf_current_filename_, filename);
            if (res < 0) {
                LOG( LOG_ERR, "renaming file \"%s\" -> \"%s\" failed erro=%u : %s\n"
                   , this->buf_current_filename_, filename, errno, strerror(errno));
            }
            else {
                this->buf_current_filename_[0] = 0;
                ++this->buf_num_file_;
                this->buf_filegen_.set_last_filename(-1u, "");
            }
        }
    }

private:
    void do_send(const uint8_t * data, size_t len) override {
        ssize_t tmpres = 0;
        if (this->buf_buf_fd == -1) {
            const char * filename = this->buf_filegen_.get(this->buf_num_file_);
            snprintf(this->buf_current_filename_, sizeof(this->buf_current_filename_),
                        "%sred-XXXXXX.tmp", filename);
            this->buf_buf_fd = ::mkostemps(this->buf_current_filename_, 4, O_WRONLY | O_CREAT);
            if (this->buf_buf_fd == -1) {
                tmpres = -1;
            }
            else {
                if (chmod(this->buf_current_filename_, this->buf_groupid_ ?
                    (S_IRUSR | S_IRGRP) : S_IRUSR) == -1) {
                LOG( LOG_ERR, "can't set file %s mod to %s : %s [%u]"
                   , this->buf_current_filename_
                   , this->buf_groupid_ ? "u+r, g+r" : "u+r"
                   , strerror(errno), errno);
                }
                this->buf_filegen_.set_last_filename(this->buf_num_file_, this->buf_current_filename_);
            }
        }
        if (tmpres != -1){
            size_t remaining_len = len;
            size_t total_sent = 0;
            while (remaining_len) {
                ssize_t ret = ::write(this->buf_buf_fd, data + total_sent, remaining_len);
                if (ret <= 0){
                    if (errno == EINTR){
                        continue;
                    }
                    tmpres = -1;
                    break;
                }
                remaining_len -= ret;
                total_sent += ret;
            }
            if (tmpres != -1){
                tmpres = total_sent;
            }
        }
    
        const ssize_t res = tmpres;
        if (res < 0) {
            this->status = false;
            if (errno == ENOSPC) {
                char message[1024];
                snprintf(message, sizeof(message), "100|unknown");
                this->authentifier->report("FILESYSTEM_FULL", message);
                errno = ENOSPC;
                throw Error(ERR_TRANSPORT_WRITE_NO_ROOM, ENOSPC);
            }
            else {
                throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
            }
        }
        this->last_quantum_sent += res;
    }
};


struct NotifyNextVideo : private noncopyable
{
    enum class reason { sequenced, external };
    virtual void notify_next_video(const timeval& now, reason) = 0;
    virtual ~NotifyNextVideo() = default;
};


class FullVideoCaptureImpl : public gdi::CaptureApi
{
    struct videocapture_OutFilenameSequenceSeekableTransport : public Transport
    {
        char buf_current_filename_[1024];
        videocapture_FilenameGenerator_PATH_FILE_EXTENSION buf_filegen_;
        int buf_buf_fd;
        unsigned buf_num_file_;
        int buf_groupid_;

    public:
        videocapture_OutFilenameSequenceSeekableTransport(
            const char * const prefix,
            const char * const filename,
            const char * const extension,
            const int groupid,
            auth_api * authentifier = nullptr)
        : buf_filegen_(prefix, filename, extension)
        , buf_buf_fd(-1)
        , buf_num_file_(0)
        , buf_groupid_(groupid)
        {
                this->buf_current_filename_[0] = 0;
                if (authentifier) {
                    this->set_authentifier(authentifier);
                }
        }

//        const videocapture_FilenameGenerator_PATH_FILE_EXTENSION * seqgen() const noexcept
//        { return &this->buf_filegen_; }

        void seek(int64_t offset, int whence) override {
            if (static_cast<off64_t>(-1) == lseek64(this->buf_buf_fd, offset, whence)){
                throw Error(ERR_TRANSPORT_SEEK_FAILED, errno);
            }
        }

        bool next() override {
            if (this->status == false) {
                throw Error(ERR_TRANSPORT_NO_MORE_DATA);
            }
            ssize_t tmpres = 1;
            if (this->buf_buf_fd != -1) {
                ::close(this->buf_buf_fd);
                this->buf_buf_fd = -1;
                // LOG(LOG_INFO, "\"%s\" -> \"%s\".", this->current_filename, this->rename_to);
                
                this->buf_filegen_.set_last_filename(-1u, "");
                const char * filename = this->buf_filegen_.get(this->buf_num_file_);
                this->buf_filegen_.set_last_filename(this->buf_num_file_, this->buf_current_filename_);
                
                const int res = ::rename(this->buf_current_filename_, filename);
                if (res < 0) {
                    LOG( LOG_ERR, "renaming file \"%s\" -> \"%s\" failed erro=%u : %s\n"
                       , this->buf_current_filename_, filename, errno, strerror(errno));
                    tmpres = 1;
                }
                else {
                    this->buf_current_filename_[0] = 0;
                    ++this->buf_num_file_;
                    this->buf_filegen_.set_last_filename(-1u, "");
                    tmpres = 0;
                }
            }
            const ssize_t res = tmpres;
            if (res) {
                this->status = false;
                if (res < 0){
                    LOG(LOG_ERR, "Write to transport failed (M): code=%d", errno);
                    throw Error(ERR_TRANSPORT_WRITE_FAILED, -res);
                }
                throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
            }
            ++this->seqno;
            return true;
        }

        void request_full_cleaning() override {
            unsigned i = this->buf_num_file_ + 1;
            while (i > 0 && !::unlink(this->buf_filegen_.get(--i))) {
            }
            if (-1 != this->buf_buf_fd) {
                ::close(this->buf_buf_fd);
                this->buf_buf_fd = -1;
            }
        }

        ~videocapture_OutFilenameSequenceSeekableTransport() {
            if (this->buf_buf_fd != -1) {
                ::close(this->buf_buf_fd);
                this->buf_buf_fd = -1;
                // LOG(LOG_INFO, "\"%s\" -> \"%s\".", this->current_filename, this->rename_to);
                
                this->buf_filegen_.set_last_filename(-1u, "");
                const char * filename = this->buf_filegen_.get(this->buf_num_file_);
                this->buf_filegen_.set_last_filename(this->buf_num_file_, this->buf_current_filename_);
                
                const int res = ::rename(this->buf_current_filename_, filename);
                if (res < 0) {
                    LOG( LOG_ERR, "renaming file \"%s\" -> \"%s\" failed erro=%u : %s\n"
                       , this->buf_current_filename_, filename, errno, strerror(errno));
                }
                else {
                    this->buf_current_filename_[0] = 0;
                    ++this->buf_num_file_;
                    this->buf_filegen_.set_last_filename(-1u, "");
                }
            }
        }

    private:
        void do_send(const uint8_t * data, size_t len) override {
            ssize_t tmpres = 0;
            if (this->buf_buf_fd == -1) {
                const char * filename = this->buf_filegen_.get(this->buf_num_file_);
                snprintf(this->buf_current_filename_, sizeof(this->buf_current_filename_),
                            "%sred-XXXXXX.tmp", filename);
                this->buf_buf_fd = ::mkostemps(this->buf_current_filename_, 4, O_WRONLY | O_CREAT);
                if (this->buf_buf_fd == -1) {
                    tmpres = -1;
                }
                else {
                    if (chmod(this->buf_current_filename_, this->buf_groupid_ ?
                        (S_IRUSR | S_IRGRP) : S_IRUSR) == -1) {
                    LOG( LOG_ERR, "can't set file %s mod to %s : %s [%u]"
                       , this->buf_current_filename_
                       , this->buf_groupid_ ? "u+r, g+r" : "u+r"
                       , strerror(errno), errno);
                    }
                    this->buf_filegen_.set_last_filename(this->buf_num_file_, this->buf_current_filename_);
                }
            }
            if (tmpres != -1){
                size_t remaining_len = len;
                size_t total_sent = 0;
                while (remaining_len) {
                    ssize_t ret = ::write(this->buf_buf_fd, data + total_sent, remaining_len);
                    if (ret <= 0){
                        if (errno == EINTR){
                            continue;
                        }
                        tmpres = -1;
                        break;
                    }
                    remaining_len -= ret;
                    total_sent += ret;
                }
                if (tmpres != -1){
                    tmpres = total_sent;
                }
            }
        
            const ssize_t res = tmpres;
            if (res < 0) {
                this->status = false;
                if (errno == ENOSPC) {
                    char message[1024];
                    snprintf(message, sizeof(message), "100|unknown");
                    this->authentifier->report("FILESYSTEM_FULL", message);
                    errno = ENOSPC;
                    throw Error(ERR_TRANSPORT_WRITE_NO_ROOM, ENOSPC);
                }
                else {
                    throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
                }
            }
            this->last_quantum_sent += res;
        }
    } trans;

    RDPDrawable & drawable;
    
    FlvParams flv_params;
    std::unique_ptr<video_recorder> recorder;
    timeval start_video_capture;
    std::chrono::microseconds inter_frame_interval;
    bool no_timestamp;

public:

    std::chrono::microseconds frame_marker_event(const timeval& now, int cursor_x, int cursor_y, bool ignore_frame_in_timeval)
    override {
        this->drawable.trace_mouse();
        if (!this->no_timestamp) {
            time_t rawtime = this->start_video_capture.tv_sec;
            tm tm_result;
            localtime_r(&rawtime, &tm_result);
            this->drawable.trace_timestamp(tm_result);
        }
        this->recorder->preparing_video_frame(true);
        if (!this->no_timestamp) { this->drawable.clear_timestamp(); }
        this->drawable.clear_mouse();
        return std::chrono::microseconds{};
    }

    std::chrono::microseconds do_snapshot(
        const timeval& now, int cursor_x, int cursor_y, bool ignore_frame_in_timeval)
    override {
        uint64_t tick = difftimeval(now, this->start_video_capture);
        uint64_t const inter_frame_interval = this->inter_frame_interval.count();
        if (tick >= inter_frame_interval) {
            auto encoding_video_frame = [this](time_t rawtime){
                this->drawable.trace_mouse();
                if (!this->no_timestamp) {
                    tm tm_result;
                    localtime_r(&rawtime, &tm_result);
                    this->drawable.trace_timestamp(tm_result);
                    this->recorder->encoding_video_frame();
                    this->drawable.clear_timestamp();
                }
                else {
                    this->recorder->encoding_video_frame();
                }
                this->drawable.clear_mouse();
            };

            if (ignore_frame_in_timeval) {
                auto const nframe = tick / inter_frame_interval;
                encoding_video_frame(this->start_video_capture.tv_sec);
                auto const usec = inter_frame_interval * nframe;
                auto sec = usec / 1000000LL;
                this->start_video_capture.tv_usec += usec - sec * inter_frame_interval;
                if (this->start_video_capture.tv_usec >= 1000000LL){
                    this->start_video_capture.tv_usec -= 1000000LL;
                    ++sec;
                }
                this->start_video_capture.tv_sec += sec;
                tick -= inter_frame_interval * nframe;
            }
            else {
                do {
                    encoding_video_frame(this->start_video_capture.tv_sec);
                    this->start_video_capture.tv_usec += inter_frame_interval;
                    if (this->start_video_capture.tv_usec >= 1000000LL){
                        this->start_video_capture.tv_sec += 1;
                        this->start_video_capture.tv_usec -= 1000000LL;
                    }
                    tick -= inter_frame_interval;
                } while (tick >= inter_frame_interval);
            }
        }
        return std::chrono::microseconds(inter_frame_interval - tick);    
    }

    std::chrono::microseconds periodic_snapshot(const timeval& now, int cursor_x, int cursor_y, bool ignore_frame_in_timeval)
    override {
        auto next_duration = this->do_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
        assert(next_duration.count() >= 0);
        return next_duration;
    }

    FullVideoCaptureImpl(const timeval & now, const char * const record_path, const char * const basename,
        const int groupid, bool no_timestamp, RDPDrawable & drawable, FlvParams flv_params)
    : trans(record_path, basename, ("." + flv_params.codec).c_str(), groupid)
    , drawable(drawable)
    , flv_params(std::move(flv_params))
    , start_video_capture(now)
    , inter_frame_interval(1000000L / this->flv_params.frame_rate)
    , no_timestamp(no_timestamp)
    {
        if (this->flv_params.verbosity) {
            LOG(LOG_INFO, "Video recording %d x %d, rate: %d, qscale: %d, brate: %d, codec: %s",
                this->flv_params.target_width, this->flv_params.target_height,
                this->flv_params.frame_rate, this->flv_params.qscale, this->flv_params.bitrate,
                this->flv_params.codec.c_str());
        }

        if (this->recorder) {
            this->recorder.reset();
            this->trans.next();
        }

        io_video_recorder_with_transport io{this->trans};
        this->recorder.reset(new video_recorder(
            io.write_fn(), io.seek_fn(), io.params(),
            drawable.width(), drawable.height(),
            drawable.pix_len(),
            drawable.data(),
            this->flv_params.bitrate,
            this->flv_params.frame_rate,
            this->flv_params.qscale,
            this->flv_params.codec.c_str(),
            this->flv_params.target_width,
            this->flv_params.target_height,
            this->flv_params.verbosity
        ));
    
        ::unlink((std::string(record_path) + basename + "." + flv_params.codec).c_str());
    }

    virtual ~FullVideoCaptureImpl() {}

    void encoding_video_frame() {
        this->recorder->encoding_video_frame();
    }

    void request_full_cleaning() {
        this->trans.request_full_cleaning();
    }
};

class SequencedVideoCaptureImpl : public gdi::CaptureApi
{
    bool ic_has_first_img = false;

public:
    std::chrono::microseconds do_snapshot(
        timeval const & now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval
    ) override
    {
        this->vc.do_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
        if (!this->ic_has_first_img) {
            return this->first_image.do_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
        }
        return this->video_sequencer.do_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
    }

    std::chrono::microseconds frame_marker_event(
        timeval const & now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval
    ) override
    {
        this->vc.frame_marker_event(now, cursor_x, cursor_y, ignore_frame_in_timeval);
        if (!this->ic_has_first_img) {
            return this->first_image.frame_marker_event(now, cursor_x, cursor_y, ignore_frame_in_timeval);
        }
        return this->video_sequencer.frame_marker_event(now, cursor_x, cursor_y, ignore_frame_in_timeval);
    }

    std::chrono::microseconds periodic_snapshot(
        timeval const & now,
        int cursor_x, int cursor_y,
        bool ignore_frame_in_timeval
    ) override
    {
        this->vc.periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
        if (!this->ic_has_first_img) {
            return this->first_image.periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
        }
        return this->video_sequencer.periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
    }

private:

    class VideoTransport final : public videocapture_OutFilenameSequenceSeekableTransport_COUNT
    {
        using transport_base = videocapture_OutFilenameSequenceSeekableTransport_COUNT;

    public:
        VideoTransport(
            const char * const record_path,
            const char * const basename,
            const char * const suffix,
            const int groupid
        )
        : transport_base(record_path, basename, suffix, groupid)
        {
            this->remove_current_path();
        }

        bool next() override {
            if (transport_base::next()) {
                this->remove_current_path();
                return true;
            }
            return false;
        }

    private:
        void remove_current_path() {
            const char * const path = this->seqgen()->get(this->get_seqno());
            ::unlink(path);
        }
    };



public:
    // first next_video is ignored
    struct FirstImage
    {
        SequencedVideoCaptureImpl & first_image_impl;

        const timeval first_image_start_capture;

        FirstImage(timeval const & now, SequencedVideoCaptureImpl & impl)
        : first_image_impl(impl)
        , first_image_start_capture(now)
        {}

        std::chrono::microseconds periodic_snapshot(
            timeval const & now,
            int cursor_x, int cursor_y,
            bool ignore_frame_in_timeval
        ) {
            // assert(now >= previous);
            auto next_duration = this->do_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
            assert(next_duration.count() >= 0);
            return next_duration;
        }

        std::chrono::microseconds frame_marker_event(
            timeval const & now,
            int cursor_x, int cursor_y,
            bool ignore_frame_in_timeval
        ) 
        {
            return this->periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
        }

        std::chrono::microseconds do_snapshot(
            const timeval& now, int x, int y, bool ignore_frame_in_timeval
        ) {
            std::chrono::microseconds ret;

            auto const duration = std::chrono::microseconds(difftimeval(now, this->first_image_start_capture));
            auto const interval = std::chrono::microseconds(std::chrono::seconds(3))/2;
            if (duration >= interval) {
                auto video_interval = first_image_impl.video_sequencer.get_interval();
                if (this->first_image_impl.ic_drawable.logical_frame_ended() || duration > std::chrono::seconds(2) || duration >= video_interval) {
                    tm ptm;
                    localtime_r(&now.tv_sec, &ptm);
                    this->first_image_impl.ic_drawable.trace_timestamp(ptm);
                    this->first_image_impl.ic_flush();
                    this->first_image_impl.ic_drawable.clear_timestamp();
                    this->first_image_impl.ic_has_first_img = true;
                    this->first_image_impl.ic_trans.next();
                    ret = video_interval;
                }
                else {
                    ret = interval / 3;
                }
            }
            else {
                ret = interval - duration;
            }

            return std::min(ret, this->first_image_impl.video_sequencer.periodic_snapshot(now, x, y, ignore_frame_in_timeval));
        }

    } first_image;

public:
    VideoTransport vc_trans;
    class VideoCapture
    {
        Transport & trans;

        FlvParams flv_params;

        RDPDrawable & drawable;
        std::unique_ptr<video_recorder> recorder;

        timeval start_video_capture;
        std::chrono::microseconds inter_frame_interval;
        bool no_timestamp;

    public:
        VideoCapture(
            const timeval & now,
            Transport & trans,
            RDPDrawable & drawable,
            bool no_timestamp,
            FlvParams flv_params)
        : trans(trans)
        , flv_params(std::move(flv_params))
        , drawable(drawable)
        , start_video_capture(now)
        , inter_frame_interval(1000000L / this->flv_params.frame_rate)
        , no_timestamp(no_timestamp)
        {
            if (this->flv_params.verbosity) {
                LOG(LOG_INFO, "Video recording %d x %d, rate: %d, qscale: %d, brate: %d, codec: %s",
                    this->flv_params.target_width, this->flv_params.target_height,
                    this->flv_params.frame_rate, this->flv_params.qscale, this->flv_params.bitrate,
                    this->flv_params.codec.c_str());
            }

            this->next_video();
        }

        void next_video() {
            if (this->recorder) {
                this->recorder.reset();
                this->trans.next();
            }

            io_video_recorder_with_transport io{this->trans};
            this->recorder.reset(new video_recorder(
                io.write_fn(), io.seek_fn(), io.params(),
                drawable.width(), drawable.height(),
                drawable.pix_len(),
                drawable.data(),
                this->flv_params.bitrate,
                this->flv_params.frame_rate,
                this->flv_params.qscale,
                this->flv_params.codec.c_str(),
                this->flv_params.target_width,
                this->flv_params.target_height,
                this->flv_params.verbosity
            ));
        }

        void preparing_video_frame() {
            this->drawable.trace_mouse();
            if (!this->no_timestamp) {
                time_t rawtime = this->start_video_capture.tv_sec;
                tm tm_result;
                localtime_r(&rawtime, &tm_result);
                this->drawable.trace_timestamp(tm_result);
            }
            this->recorder->preparing_video_frame(true);
            if (!this->no_timestamp) { this->drawable.clear_timestamp(); }
            this->drawable.clear_mouse();
        }

        void encoding_video_frame() {
            this->recorder->encoding_video_frame();
        }

        std::chrono::microseconds frame_marker_event(
            const timeval& /*now*/, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/
        ) {
            this->preparing_video_frame();
            return std::chrono::microseconds{};
        }

        std::chrono::microseconds do_snapshot(
            const timeval& now, int /*cursor_x*/, int /*cursor_y*/, bool ignore_frame_in_timeval
        ) {
            uint64_t tick = difftimeval(now, this->start_video_capture);
            uint64_t const inter_frame_interval = this->inter_frame_interval.count();
            if (tick >= inter_frame_interval) {
                auto encoding_video_frame = [this](time_t rawtime){
                    this->drawable.trace_mouse();
                    if (!this->no_timestamp) {
                        tm tm_result;
                        localtime_r(&rawtime, &tm_result);
                        this->drawable.trace_timestamp(tm_result);
                        this->recorder->encoding_video_frame();
                        this->drawable.clear_timestamp();
                    }
                    else {
                        this->recorder->encoding_video_frame();
                    }
                    this->drawable.clear_mouse();
                };

                if (ignore_frame_in_timeval) {
                    auto const nframe = tick / inter_frame_interval;
                    encoding_video_frame(this->start_video_capture.tv_sec);
                    auto const usec = inter_frame_interval * nframe;
                    auto sec = usec / 1000000LL;
                    this->start_video_capture.tv_usec += usec - sec * inter_frame_interval;
                    if (this->start_video_capture.tv_usec >= 1000000LL){
                        this->start_video_capture.tv_usec -= 1000000LL;
                        ++sec;
                    }
                    this->start_video_capture.tv_sec += sec;
                    tick -= inter_frame_interval * nframe;
                }
                else {
                    do {
                        encoding_video_frame(this->start_video_capture.tv_sec);
                        this->start_video_capture.tv_usec += inter_frame_interval;
                        if (this->start_video_capture.tv_usec >= 1000000LL){
                            this->start_video_capture.tv_sec += 1;
                            this->start_video_capture.tv_usec -= 1000000LL;
                        }
                        tick -= inter_frame_interval;
                    } while (tick >= inter_frame_interval);
                }
            }

            return std::chrono::microseconds(inter_frame_interval - tick);
        }
        
        std::chrono::microseconds periodic_snapshot(
            timeval const & now,
            int cursor_x, int cursor_y,
            bool ignore_frame_in_timeval
        ) {
            // assert(now >= previous);
            auto next_duration = this->do_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
            assert(next_duration.count() >= 0);
            return next_duration;
        }

    } vc;
 
    struct videocapture_OutFilenameSequenceTransport_COUNT : public Transport
    {
        char buf_current_filename_[1024];
        videocapture_FilenameGenerator_PATH_FILE_COUNT_EXTENSION buf_filegen_;
        int buf_buf_fd;
        unsigned buf_num_file_;
        int buf_groupid_;

        videocapture_OutFilenameSequenceTransport_COUNT(
            const char * const prefix,
            const char * const filename,
            const char * const extension,
            const int groupid,
            auth_api * authentifier)
        : buf_filegen_(prefix, filename, extension)
            , buf_buf_fd(-1)
            , buf_num_file_(0)
            , buf_groupid_(groupid)
        {
                this->buf_current_filename_[0] = 0;
                if (authentifier) {
                    this->set_authentifier(authentifier);
                }
        }

        const videocapture_FilenameGenerator_PATH_FILE_COUNT_EXTENSION * seqgen() const noexcept
        { return &(this->buf_filegen_); }

        bool next() override {
            if (this->status == false) {
                throw Error(ERR_TRANSPORT_NO_MORE_DATA);
            }
            ssize_t tmpres = 1;
            if (this->buf_buf_fd != -1) {
                ::close(this->buf_buf_fd);
                this->buf_buf_fd = -1;
                // LOG(LOG_INFO, "\"%s\" -> \"%s\".", this->current_filename, this->rename_to);
                this->buf_filegen_.set_last_filename(-1u, "");
                const char * filename = this->buf_filegen_.get(this->buf_num_file_);
                this->buf_filegen_.set_last_filename(this->buf_num_file_, this->buf_current_filename_);

                const int res = ::rename(this->buf_current_filename_, filename);
                if (res < 0) {
                    LOG( LOG_ERR, "renaming file \"%s\" -> \"%s\" failed erro=%u : %s\n"
                       , this->buf_current_filename_, filename, errno, strerror(errno));
                }
                else {
                    this->buf_current_filename_[0] = 0;
                    ++this->buf_num_file_;
                    this->buf_filegen_.set_last_filename(-1u, "");
                    tmpres = 0;
                }
            }
            const ssize_t res = tmpres;
            if (res) {
                this->status = false;
                if (res < 0){
                    LOG(LOG_ERR, "Write to transport failed (M): code=%d", errno);
                    throw Error(ERR_TRANSPORT_WRITE_FAILED, -res);
                }
                throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
            }
            ++this->seqno;
            return true;
        }

        void request_full_cleaning() override {
            unsigned i = this->buf_num_file_ + 1;
            while (i > 0 && !::unlink(this->buf_filegen_.get(--i))) {
            }
            if (-1 != this->buf_buf_fd) {
                ::close(this->buf_buf_fd);
                this->buf_buf_fd = -1;
            }
        }

        ~videocapture_OutFilenameSequenceTransport_COUNT() {
            if (this->buf_buf_fd != -1) {
                ::close(this->buf_buf_fd);
                this->buf_buf_fd = -1;
                // LOG(LOG_INFO, "\"%s\" -> \"%s\".", this->buf_current_filename, this->buf_rename_to);
                this->buf_filegen_.set_last_filename(-1u, "");
                const char * filename = this->buf_filegen_.get(this->buf_num_file_);
                this->buf_filegen_.set_last_filename(this->buf_num_file_, this->buf_current_filename_);
                const int res = ::rename(this->buf_current_filename_, filename);
                if (res < 0) {
                    LOG( LOG_ERR, "renaming file \"%s\" -> \"%s\" failed erro=%u : %s\n"
                       , this->buf_current_filename_, filename, errno, strerror(errno));
                }
                else {
                    this->buf_current_filename_[0] = 0;
                    ++this->buf_num_file_;
                    this->buf_filegen_.set_last_filename(-1u, "");
                }
            }
        }

    private:

        void do_send(const uint8_t * data, size_t len) override {
        {
            if (this->buf_buf_fd == -1) {
                const char * filename = this->buf_filegen_.get(this->buf_num_file_);
                snprintf(this->buf_current_filename_, sizeof(this->buf_current_filename_),
                            "%sred-XXXXXX.tmp", filename);
                this->buf_buf_fd = ::mkostemps(this->buf_current_filename_, 4, O_WRONLY | O_CREAT);
                if (this->buf_buf_fd >= 0) {
                    }
                    if (chmod(this->buf_current_filename_, this->buf_groupid_
                        ? (S_IRUSR | S_IRGRP) : S_IRUSR) == -1) {
                        LOG( LOG_ERR, "can't set file %s mod to %s : %s [%u]"
                           , this->buf_current_filename_
                           , this->buf_groupid_ ? "u+r, g+r" : "u+r"
                           , strerror(errno), errno);
                           // TODO: this should be an error
                    }
                    this->buf_filegen_.set_last_filename(this->buf_num_file_, this->buf_current_filename_);
                }
            }

            int tmpres = -1;
            if (this->buf_buf_fd >= 0){
                tmpres = 0;
                size_t remaining_len = len;
                size_t total_sent = 0;
                while (remaining_len) {
                    ssize_t ret = ::write(this->buf_buf_fd, data + total_sent, remaining_len);
                    if (ret <= 0){
                        if (errno == EINTR){
                            continue;
                        }
                        tmpres = -1;
                        break;
                    }
                    remaining_len -= ret;
                    total_sent += ret;
                }
                if (tmpres == 0){
                    tmpres = total_sent;
                }
            }

            const ssize_t res = tmpres;
            if (res < 0) {
                this->status = false;
                if (errno == ENOSPC) {
                    char message[1024];
                    snprintf(message, sizeof(message), "100|unknown");
                    this->authentifier->report("FILESYSTEM_FULL", message);
                    errno = ENOSPC;
                    throw Error(ERR_TRANSPORT_WRITE_NO_ROOM, ENOSPC);
                }
                else {
                    throw Error(ERR_TRANSPORT_WRITE_FAILED, errno);
                }
            }
            this->last_quantum_sent += res;
        }

    } ic_trans;

    unsigned ic_zoom_factor;
    unsigned ic_scaled_width;
    unsigned ic_scaled_height;

    /* const */ RDPDrawable & ic_drawable;

    private:
        std::unique_ptr<uint8_t[]> ic_scaled_buffer;

    public:
    void zoom(unsigned percent) {
        percent = std::min(percent, 100u);
        const unsigned zoom_width = (this->ic_drawable.width() * percent) / 100;
        const unsigned zoom_height = (this->ic_drawable.height() * percent) / 100;
        this->ic_zoom_factor = percent;
        this->ic_scaled_width = (zoom_width + 3) & 0xFFC;
        this->ic_scaled_height = zoom_height;
        if (this->ic_zoom_factor != 100) {
            this->ic_scaled_buffer.reset(new uint8_t[this->ic_scaled_width * this->ic_scaled_height * 3]);
        }
    }

    void ic_flush() {
        if (this->ic_zoom_factor == 100) {
            this->dump24();
        }
        else {
            this->scale_dump24();
        }
    }

    void dump24() {
        ::transport_dump_png24(
            this->ic_trans, this->ic_drawable.data(),
            this->ic_drawable.width(), this->ic_drawable.height(),
            this->ic_drawable.rowsize(), true);
    }

    void scale_dump24() {
        scale_data(
            this->ic_scaled_buffer.get(), this->ic_drawable.data(),
            this->ic_scaled_width, this->ic_drawable.width(),
            this->ic_scaled_height, this->ic_drawable.height(),
            this->ic_drawable.rowsize());
        ::transport_dump_png24(
            this->ic_trans, this->ic_scaled_buffer.get(),
            this->ic_scaled_width, this->ic_scaled_height,
            this->ic_scaled_width * 3, false);
    }

    class VideoSequencer : public gdi::CaptureApi
    {
        timeval start_break;
        std::chrono::microseconds break_interval;

    protected:
        SequencedVideoCaptureImpl & impl;

    public:
        VideoSequencer(const timeval & now, std::chrono::microseconds break_interval, SequencedVideoCaptureImpl & impl)
        : start_break(now)
        , break_interval(break_interval)
        , impl(impl)
        {}

        std::chrono::microseconds get_interval() const {
            return this->break_interval;
        }

        void reset_now(const timeval& now) {
            this->start_break = now;
        }

        std::chrono::microseconds do_snapshot(
            const timeval& now, int /*cursor_x*/, int /*cursor_y*/, bool /*ignore_frame_in_timeval*/
        ) override {
            assert(this->break_interval.count());
            auto const interval = difftimeval(now, this->start_break);
            if (interval >= uint64_t(this->break_interval.count())) {
                this->impl.next_video_impl(now, NotifyNextVideo::reason::sequenced);
                this->start_break = now;
            }
            return this->break_interval;
        }

        std::chrono::microseconds periodic_snapshot(
            timeval const & now,
            int cursor_x, int cursor_y,
            bool ignore_frame_in_timeval
        ) {
            // assert(now >= previous);
            auto next_duration = this->do_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
            assert(next_duration.count() >= 0);
            return next_duration;
        }

        std::chrono::microseconds frame_marker_event(
            timeval const & now,
            int cursor_x, int cursor_y,
            bool ignore_frame_in_timeval
        ) 
        {
            return this->periodic_snapshot(now, cursor_x, cursor_y, ignore_frame_in_timeval);
        }

    } video_sequencer;

    NotifyNextVideo & next_video_notifier;

    void next_video_impl(const timeval& now, NotifyNextVideo::reason reason) {
        this->video_sequencer.reset_now(now);
        if (!this->ic_has_first_img) {
            tm ptm;
            localtime_r(&now.tv_sec, &ptm);
            this->ic_drawable.trace_timestamp(ptm);
            this->ic_flush();
            this->ic_drawable.clear_timestamp();
            this->ic_has_first_img = true;
            this->ic_trans.next();
        }
        this->vc.next_video();
        tm ptm;
        localtime_r(&now.tv_sec, &ptm);
        this->ic_drawable.trace_timestamp(ptm);
        this->ic_flush();
        this->ic_drawable.clear_timestamp();
        this->ic_trans.next();
        this->next_video_notifier.notify_next_video(now, reason);
    }

public:
    SequencedVideoCaptureImpl(
        const timeval & now,
        const char * const record_path,
        const char * const basename,
        const int groupid,
        bool no_timestamp,
        unsigned image_zoom,
        /* const */RDPDrawable & drawable,
        FlvParams flv_params,
        std::chrono::microseconds video_interval,
        NotifyNextVideo & next_video_notifier)
    : first_image(now, *this)

    , vc_trans(record_path, basename, ("." + flv_params.codec).c_str(), groupid)
    , vc(now, this->vc_trans, drawable, no_timestamp, std::move(flv_params))
    , ic_trans(record_path, basename, ".png", groupid, nullptr)
    , ic_zoom_factor(std::min(image_zoom, 100u))
    , ic_scaled_width(drawable.width())
    , ic_scaled_height(drawable.height())
    , ic_drawable(drawable)
    , video_sequencer(
        now, video_interval > std::chrono::microseconds(0) ? video_interval : std::chrono::microseconds::max(), *this)
    , next_video_notifier(next_video_notifier)
    {
        const unsigned zoom_width = (this->ic_drawable.width() * this->ic_zoom_factor) / 100;
        const unsigned zoom_height = (this->ic_drawable.height() * this->ic_zoom_factor) / 100;
        this->ic_scaled_width = (zoom_width + 3) & 0xFFC;
        this->ic_scaled_height = zoom_height;
        if (this->ic_zoom_factor != 100) {
            this->ic_scaled_buffer.reset(new uint8_t[this->ic_scaled_width * this->ic_scaled_height * 3]);
        }
    }

    void next_video(const timeval& now) {
        this->next_video_impl(now, NotifyNextVideo::reason::external);
    }

    void encoding_video_frame() {
        this->vc.encoding_video_frame();
    }

    void request_full_cleaning() {
        this->vc_trans.request_full_cleaning();
        this->ic_trans.request_full_cleaning();
    }
};

