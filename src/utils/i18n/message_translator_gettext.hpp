#pragma once

#include <string>

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
        zstring_view get_translated_text(const char *text) const noexcept;

    private :
        bool _clearable_context;
    };
}
