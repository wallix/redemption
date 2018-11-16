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
*   Copyright (C) Wallix 2010-2017
*   Author(s): Christophe Grosjean
*/

#pragma once

struct MetaParams
{
    enum class EnableSessionLog : bool { No, Yes } enable_session_log;
    enum class HideNonPrintable : bool { No, Yes } hide_non_printable;

    enum class LogClipboardActivities : bool { No, Yes } log_clipboard_activities;
    enum class LogFileSystemActivities : bool { No, Yes } log_file_system_activities;

    enum class LogOnlyRelevantClipboardActivities : bool { No, Yes } log_only_relevant_clipboard_activities;
};
