/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2017
   Author(s): Jonathan Poelen
*/

#include "capture/capture_params.hpp"
#include "capture/full_video_params.hpp"
#include "capture/video_capture.hpp"
#include "utils/fileutils.hpp"
#include "utils/set_exception_handler_pretty_message.hpp"

#include <iostream>

int main(int ac, char ** av)
{
    if (ac <= 1 || (av[1][0] == '-' && av[1][1] == 'h' && !av[1][2])) {
        std::cout << av[0] << " [outfile = ./test.mp4]\n";
        return 0;
    }

    set_exception_handler_pretty_message();

    char path[1024]; path[0] = 0;
    char basename[1024]; basename[0] = 0;
    char extension[8]; extension[0] = 0; // and codec
    char const * const filename = ac ? av[1] : "./test.mp4";

    canonical_path(
        filename,
        path, sizeof(path),
        basename, sizeof(basename),
        extension, sizeof(extension));

    char const * codec = extension[0] && extension[0] == '.' ? extension+1 : "mp4";

    timeval now {1353055800, 0};
    RDPDrawable drawable(800, 600);
    VideoParams video_params{
        Level::medium, drawable.width(), drawable.height(),
        25, 15, 100000, codec, false, false, false, std::chrono::microseconds{2 * 1000000l}, 0};
    CaptureParams capture_params{
        now, basename, nullptr, path, 0 /* groupid */, nullptr, SmartVideoCropping::disable, 0};
    FullVideoCaptureImpl capture(
        capture_params, drawable, drawable, video_params, FullVideoParams{false});

    Rect screen(0, 0, drawable.width(), drawable.height());
    auto const color_cxt = gdi::ColorCtx::depth24();
    drawable.draw(RDPOpaqueRect(screen, encode_color24()(BLUE)), screen, color_cxt);
    uint64_t usec = now.tv_sec * 1000000LL + now.tv_usec;
    Rect r(10, 10, 50, 50);
    int vx = 5;
    int vy = 4;
    auto const color = encode_color24()(WABGREEN);
    bool ignore_frame_in_timeval = false;
    for (size_t x = 0; x < 200; x++) {
        r.y += vy;
        drawable.draw(RDPOpaqueRect(r, encode_color24()(BLUE)),  screen, color_cxt);
        r.x += vx;
        drawable.draw(RDPOpaqueRect(r, color), screen, color_cxt);
        usec += 40000LL;
        now.tv_sec  = usec / 1000000LL;
        now.tv_usec = (usec % 1000000LL);
        drawable.set_mouse_cursor_pos(r.x + 10, r.y + 10);
        capture.periodic_snapshot(now,  r.x + 10, r.y + 10, ignore_frame_in_timeval);
        if ((r.x + r.cx >= drawable.width())  || (r.x < 0)) { vx = -vx; }
        if ((r.y + r.cy >= drawable.height()) || (r.y < 0)) { vy = -vy; }
    }
}
