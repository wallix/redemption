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
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#include "core/report_error.hpp"
#include "utils/log.hpp"


//void LogReporter::operator()(char const * reason, char const * message)
//{
//    LOG(LOG_ERR, "%s:%s", reason, message);
//}

//Error ReportError::NullImpl::get_error(Error err)
//{
//    report_and_transform_error(err, LogReporter{});
//    return err;
//}

//ReportError report_error_from_reporter(AuthApi & sesman)
//{
//    return ReportError([&sesman](Error error) {
//        report_and_transform_error(error, ReportMessageReporter{sesman});
//        return error;
//    });
//}

//ReportError report_error_from_reporter(AuthApi * sesman)
//{
//    return sesman ? report_error_from_reporter(*sesman) : ReportError();
//}
