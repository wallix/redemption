/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARIO *ICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

   Template for new SQ_One sequence class

*/

#ifndef _REDEMPTION_LIBS_SQ_ONE_H_
#define _REDEMPTION_LIBS_SQ_ONE_H_

#include "rio_constants.h"

extern "C" {
    struct SQOne {
        RIO * trans;
    };

    RIO_ERROR sq_m_SQOne_constructor(SQOne * self, RIO * trans)
    {
        self->trans = trans;
        return RIO_ERROR_OK;
    }

    RIO_ERROR sq_m_SQOne_destructor(SQOne * self)
    {
        return RIO_ERROR_OK;
    }

    RIO * sq_m_SQOne_get_trans(SQOne * self, RIO_ERROR * status)
    {
        return self->trans;
    }

    RIO_ERROR sq_m_SQOne_next(SQOne * self)
    {
        return RIO_ERROR_OK;
    }
};

#endif

