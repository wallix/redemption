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
  Copyright (C) Wallix 2020
  Author(s): Wallix Team
*/

#pragma once

#include "utils/sugar/zstring_view.hpp"

namespace i18n
{
    class MessageTranslatorGettext
    {
    public :
        MessageTranslatorGettext();
        ~MessageTranslatorGettext();
        void clear_context();
        void set_context(zstring_view locale_name);

        [[nodiscard]]
        zstring_view get_translated_text(zstring_view text) const noexcept;

    private :
        bool _clearable_context;
    };
} // namespace i18n
