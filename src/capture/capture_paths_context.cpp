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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#include "capture/capture_paths_context.hpp"
#include "core/date_dir_from_filename.hpp"
#include "utils/log.hpp"
#include "utils/sugar/algostring.hpp"


CapturePathsContext::CapturePathsContext(std::string_view record_path, std::string_view hash_path, std::string_view log_path)
{
    DateDirFromFilename d(log_path);
    if (!d.has_date()) {
        LOG(LOG_WARNING, "CapturePathsContext: failed to extract date");
    }

    str_append(this->hash_path, hash_path, d.date_path());
    str_append(this->record_path, record_path, d.date_path());
}
