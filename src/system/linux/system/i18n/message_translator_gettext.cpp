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

#include "message_translator_gettext.hpp"

#include "core/app_path.hpp"
#include "utils/fileutils.hpp"
#include "utils/strutils.hpp"
#include "utils/log.hpp"

#include <cerrno>
#include <clocale>

#include <libintl.h>


namespace
{
    constexpr zstring_view GETTEXTINSTALL_DIRNAME = "/locale/"_zv;
    constexpr zstring_view CATEGORY_DIRNAME = "/LC_MESSAGES/"_zv;
    constexpr zstring_view DOMAIN_NAME = "redemption"_zv;
    constexpr zstring_view DOMAIN_NAME_EXT = ".mo"_zv;
    constexpr const char *CODESET = "UTF-8";
    constexpr const char *LOCALE = "C.UTF-8";
    constexpr const char *LANGUAGE_ENV_VAR = "LANGUAGE";

    [[nodiscard]]
    inline bool
    check_directory(const std::string& directory_path,
                    zstring_view locale_name)
    {
        if (!dir_exist(directory_path)) {
            LOG(LOG_WARNING,
                "need %s directory for messages with \"%s\" locale",
                directory_path, locale_name);
            return false;
        }
        return true;
    }

    [[nodiscard]]
    inline bool
    check_domain_name_file_path(const std::string& domain_name_file_path,
                                zstring_view locale_name)
    {
        if (!file_exist(domain_name_file_path))
        {
            LOG(LOG_WARNING,
                "need domain name file for messages with \"%s\" locale",
                locale_name);
            return false;
        }

        if (dir_exist(domain_name_file_path))
        {
            LOG(LOG_WARNING,
                "domain name must be a file for messages with \"%s\" locale",
                locale_name);
            return false;
        }
        return true;
    }
}

namespace i18n
{
    MessageTranslatorGettext::MessageTranslatorGettext() :
        _clearable_context(false)
    { }

    MessageTranslatorGettext::~MessageTranslatorGettext()
    {
        clear_context();
    }

    void MessageTranslatorGettext::clear_context()
    {
        if (!_clearable_context)
        {
            return;
        }

        if (!std::setlocale(LC_MESSAGES, ""))
        {
            LOG(LOG_ERR, "cannot reset LC_MESSAGES environment variable");
        }

        if (unsetenv(LANGUAGE_ENV_VAR) == -1)
        {
            if (errno == ENOMEM)
            {
                LOG(LOG_ERR, "unsetenv() error (errno : %d)", errno);
                throw std::bad_alloc();
            }
            LOG(LOG_WARNING,
                "clear \"%s\" environment variable not successful",
                LANGUAGE_ENV_VAR);
        }

        if (!bindtextdomain("", ""))
        {
            if (errno == ENOMEM)
            {
                LOG(LOG_ERR, "bindtextdomain() error (errno : %d)", errno);
                throw std::bad_alloc();
            }
        }

        if (!bind_textdomain_codeset("", ""))
        {
            if (errno == ENOMEM)
            {
                LOG(LOG_ERR,
                    "bind_textdomain_codeset() error (errno : %d)",
                    errno);
                throw std::bad_alloc();
            }
        }

        if (!textdomain(""))
        {
            if (errno == ENOMEM)
            {
                LOG(LOG_ERR, "textdomain() error (errno : %d)", errno);
                throw std::bad_alloc();
            }
        }
        LOG(LOG_INFO, "i18n context is cleared");
        _clearable_context = false;
    }

    void MessageTranslatorGettext::set_context(zstring_view locale_name)
    {
        std::string locale_path = str_concat(app_path(AppPath::Share),
                                             GETTEXTINSTALL_DIRNAME);

        if (!check_directory(locale_path, locale_name)) {
            return;
        }

        std::string target_locale_path = str_concat(locale_path, locale_name);

        if (!check_directory(target_locale_path, locale_name)) {
            return;
        }

        std::string category_path = str_concat(target_locale_path,
                                               CATEGORY_DIRNAME);

        if (!check_directory(category_path, locale_name)) {
            return;
        }

        if (auto domain_name_file_path = str_concat(category_path,
                                                    DOMAIN_NAME,
                                                    DOMAIN_NAME_EXT);
            !check_domain_name_file_path(domain_name_file_path, locale_name)
        ) {
            return;
        }

        if (!std::setlocale(LC_MESSAGES, LOCALE)) {
            LOG(LOG_ERR,
                "cannot set LC_MESSAGES environment variable "
                "with locale for use GNU gettext framework");
            return;
        }

        if (setenv(LANGUAGE_ENV_VAR, locale_name.c_str(), 1) == -1) {
            LOG(LOG_ERR,
                "cannot set LANGUAGE environment variable "
                "with locale \"%s\" for use GNU gettext framework",
                locale_name);
            return;
        }

        if (!bindtextdomain(DOMAIN_NAME.c_str(), locale_path.c_str())) {
            LOG(LOG_ERR,
                "cannot set directory for domain name "
                "with locale \"%s\" for use GNU gettext framework",
                locale_name);
            return;
        }

        if (!bind_textdomain_codeset(DOMAIN_NAME.c_str(), CODESET)) {
            LOG(LOG_ERR,
                "cannot set codeset for domain name "
                "with locale \"%s\" for use GNU gettext framework",
                locale_name);
            return;
        }

        if (!textdomain(DOMAIN_NAME.c_str())) {
            LOG(LOG_ERR,
                "cannot set domain name with locale \"%s\" "
                "for use GNU gettext framework",
                locale_name);
            return;
        }

        LOG(LOG_INFO, "i18n context is set for \"%s\" locale", locale_name);
        _clearable_context = true;
    }

    zstring_view
    MessageTranslatorGettext::get_translated_text(zstring_view text)
        const noexcept
    {
        return zstring_view::from_null_terminated(gettext(text));
    }
}
