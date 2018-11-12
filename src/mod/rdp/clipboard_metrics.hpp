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
  Copyright (C) Wallix 2018

 clipboard_metrics.hpp

  Created on: 9 nov. 2018
  Author: Christophe Grosjean
 */


#include "core/RDP/clipboard.hpp"

namespace RDPECLIP {

inline static const char * get_FormatId_name(uint32_t FormatId) {
    switch (FormatId) {
        case CF_TEXT:            return "CF_TEXT";
        case CF_BITMAP:          return "CF_BITMAP";
        case CF_METAFILEPICT:    return "CF_METAFILEPICT";
        case CF_SYLK:            return "CF_SYLK";
        case CF_DIF:             return "CF_DIF";
        case CF_TIFF:            return "CF_TIFF";
        case CF_OEMTEXT:         return "CF_OEMTEXT";
        case CF_DIB:             return "CF_DIB";
        case CF_PALETTE:         return "CF_PALETTE";
        case CF_PENDATA:         return "CF_PENDATA";
        case CF_RIFF:            return "CF_RIFF";
        case CF_WAVE:            return "CF_WAVE";
        case CF_UNICODETEXT:     return "CF_UNICODETEXT";
        case CF_ENHMETAFILE:     return "CF_ENHMETAFILE";
        case CF_HDROP:           return "CF_HDROP";
        case CF_LOCALE:          return "CF_LOCALE";
        case CF_DIBV5:           return "CF_DIBV5";
        case CF_OWNERDISPLAY:    return "CF_OWNERDISPLAY";
        case CF_DSPTEXT:         return "CF_DSPTEXT";
        case CF_DSPBITMAP:       return "CF_DSPBITMAP";
        case CF_DSPMETAFILEPICT: return "CF_DSPMETAFILEPICT";
        case CF_DSPENHMETAFILE:  return "CF_DSPENHMETAFILE";
        case CF_PRIVATEFIRST:    return "CF_PRIVATEFIRST";
        case CF_PRIVATELAST:     return "CF_PRIVATELAST";
        case CF_GDIOBJFIRST:     return "CF_GDIOBJFIRST";
        case CF_GDIOBJLAST:      return "CF_GDIOBJLAST";
    }

    return "<unknown>";
}

class FormatName {
    uint32_t    formatId_ = 0;
    std::string format_name_;

public:
    explicit FormatName(uint32_t formatId, const char * format_name)
    : formatId_(formatId)
    , format_name_(format_name) {}

    explicit FormatName(uint32_t formatId, std::string && format_name)
    : formatId_(formatId)
    , format_name_(std::move(format_name)) {}

    uint32_t formatId() const { return this->formatId_; }

    void formatId(uint32_t formatId) { this->formatId_ = formatId; }

    const char * format_name() const { return this->format_name_.c_str(); }

    void format_name(const char * format_name) { this->format_name_ = format_name; }

    size_t format_name_length() const { return this->format_name_.length(); }

    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length,
            "{formatId=%s(%u) formatName=\"%s\"}",
            get_FormatId_name(this->formatId_), this->formatId_, this->format_name_.c_str());
        length += ((result < size - length) ? result : (size - length - 1));

        return length;
    }

    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};  // class FormatName


class FormatListPDUEx {
    std::vector<class FormatName> format_names;

public:
    void emit(OutStream & stream, bool use_long_format_names) const {
        if (use_long_format_names) {
            for (auto & format_name : this->format_names) {
                stream.out_uint32_le(format_name.formatId());

                const size_t formatName_unicode_data_size =
                    (format_name.format_name_length() + 1) * maximum_length_of_utf8_character_in_bytes;

                std::unique_ptr<uint8_t[]> formatName_unicode_data = std::make_unique<uint8_t[]>(
                    formatName_unicode_data_size);

                const size_t size_of_formatName_unicode_data = ::UTF8toUTF16(
                    byte_ptr_cast(format_name.format_name()),
                    formatName_unicode_data.get(), formatName_unicode_data_size);
                assert(size_of_formatName_unicode_data + 2 < formatName_unicode_data_size);

                if (size_of_formatName_unicode_data < formatName_unicode_data_size) {
                    ::memset(formatName_unicode_data.get() + size_of_formatName_unicode_data,
                        0, formatName_unicode_data_size - size_of_formatName_unicode_data);
                }

                stream.out_copy_bytes(formatName_unicode_data.get(),
                    size_of_formatName_unicode_data + 2);
            }
        }
        else {
            bool   all_format_names_are_ASCII_strings = true;
            size_t max_format_name_len_in_char        = 0;
            for (auto & format_name : this->format_names) {
                if (!::is_ASCII_string(byte_ptr_cast(format_name.format_name()))) {
                    all_format_names_are_ASCII_strings = false;
                }

                const size_t format_name_len_in_char = ::UTF8StrLenInChar(byte_ptr_cast(format_name.format_name()));
                if (max_format_name_len_in_char < format_name_len_in_char) {
                    max_format_name_len_in_char = format_name_len_in_char;
                }
            }

            if (!all_format_names_are_ASCII_strings ||
                (max_format_name_len_in_char < (32 /* formatName(32) */ / sizeof(uint16_t))))
            {
                // Unicode

                for (auto & format_name : this->format_names) {
                    stream.out_uint32_le(format_name.formatId());

                    constexpr size_t formatName_unicode_data_size = 32; // formatName(32)

                    uint8_t formatName_unicode_data[formatName_unicode_data_size];

                    const size_t size_of_formatName_unicode_data = ::UTF8toUTF16(
                        byte_ptr_cast(format_name.format_name()),
                        formatName_unicode_data, formatName_unicode_data_size - sizeof(uint16_t));
                    assert(size_of_formatName_unicode_data + 2 <= formatName_unicode_data_size);

                    if (size_of_formatName_unicode_data < formatName_unicode_data_size) {
                        ::memset(formatName_unicode_data + size_of_formatName_unicode_data,
                            0, formatName_unicode_data_size - size_of_formatName_unicode_data);
                    }

                    stream.out_copy_bytes(formatName_unicode_data,
                        formatName_unicode_data_size);
                }
            }
            else {
                // ASCII

                for (auto & format_name : this->format_names) {
                    stream.out_uint32_le(format_name.formatId());

                    const size_t size_of_formatName_ASCII_data = std::min<size_t>(
                            format_name.format_name_length(),
                            32 /* formatName(32) */ - sizeof(uint8_t));

                    stream.out_copy_bytes(format_name.format_name(),
                        size_of_formatName_ASCII_data);

                    stream.out_clear_bytes(32 /* formatName(32) */ - size_of_formatName_ASCII_data);
                }
            }
        }
    }

    void recv(InStream & stream, bool use_long_format_names, bool in_ASCII_8_) {
        this->format_names.clear();

        if (use_long_format_names) {
            constexpr size_t min_long_format_name_data_length = 6;  // formatId(4) + wszFormatName(variable, min = "\x00\x00" => 2)
            while (stream.in_remain() >= min_long_format_name_data_length) {
                const uint32_t formatId = stream.in_uint32_le();

                const size_t format_name_UTF16_length = ::UTF16StrLen(stream.get_current());

                const size_t formatName_UTF8_data_size =
                    (format_name_UTF16_length + 1) * maximum_length_of_utf8_character_in_bytes;

                std::string format_name(formatName_UTF8_data_size, 0);

                const size_t size_of_formatName_UTF8_data = ::UTF16toUTF8(
                    stream.get_current(), (format_name_UTF16_length + 1),
                    byte_ptr_cast(format_name.data()), formatName_UTF8_data_size);
                assert(size_of_formatName_UTF8_data + 1 < formatName_UTF8_data_size);

                this->format_names.emplace_back(
                        formatId,
                        std::move(format_name)
                    );

                stream.in_skip_bytes((format_name_UTF16_length + 1) * sizeof(uint16_t));
            }
        }
        else {
            constexpr size_t short_format_name_data_length = 36;  // formatId(4) + formatName(32)
            if (in_ASCII_8_) {
                while (stream.in_remain() >= short_format_name_data_length) {
                    const uint32_t formatId = stream.in_uint32_le();

                    this->format_names.emplace_back(
                            formatId,
                            char_ptr_cast(stream.get_current())
                        );

                    stream.in_skip_bytes(short_format_name_data_length - 4 /* formatId(4) */);
                }
            }
            else {
                while (stream.in_remain() >= short_format_name_data_length) {
                    const uint32_t formatId = stream.in_uint32_le();

                    const size_t format_name_UTF16_length = (32 /* formatName(32) */ / sizeof(uint16_t));

                    const size_t formatName_UTF8_data_size =
                        format_name_UTF16_length * maximum_length_of_utf8_character_in_bytes;

                    std::string format_name(formatName_UTF8_data_size, 0);

                    const size_t size_of_formatName_UTF8_data = ::UTF16toUTF8(
                        stream.get_current(), format_name_UTF16_length,
                        byte_ptr_cast(format_name.data()), formatName_UTF8_data_size);
                    assert(size_of_formatName_UTF8_data + 1 < formatName_UTF8_data_size);

                    this->format_names.emplace_back(
                            formatId,
                            std::move(format_name)
                        );

                    stream.in_skip_bytes(short_format_name_data_length - 4 /* formatId(4) */);
                }
            }
        }
    }

    void add_format_name(uint32_t formatId, const char * format_name = "") {
        this->format_names.emplace_back(formatId, format_name);
    }

    size_t num_format_names() const { return this->format_names.size(); }

    FormatName const & format_name(size_t idx_format_name) const { return this->format_names[idx_format_name]; }

    size_t size(bool use_long_format_names) const {
        size_t sz = 0;

        if (use_long_format_names) {
            for (auto & format_name : this->format_names) {
                sz += 4;    // formatId(4)

                const size_t formatName_unicode_data_size =
                    (format_name.format_name_length() + 1) * maximum_length_of_utf8_character_in_bytes;

                std::unique_ptr<uint8_t[]> formatName_unicode_data = std::make_unique<uint8_t[]>(
                    formatName_unicode_data_size);

                const size_t size_of_formatName_unicode_data = ::UTF8toUTF16(
                    byte_ptr_cast(format_name.format_name()),
                    formatName_unicode_data.get(), formatName_unicode_data_size);
                sz += (size_of_formatName_unicode_data + 2);    // wszFormatName (variable)
            }
        }
        else {
            sz = this->format_names.size() * (
                      4     // formatId(4)
                    + 32    // formatName(32)
                );
        }

        return sz;
    }

    bool will_be_sent_in_ASCII_8(bool use_long_format_names) {
        if (!use_long_format_names) {
            bool   all_format_names_are_ASCII_strings = true;
            size_t max_format_name_len_in_char        = 0;
            for (auto & format_name : this->format_names) {
                if (!::is_ASCII_string(byte_ptr_cast(format_name.format_name()))) {
                    all_format_names_are_ASCII_strings = false;
                }

                const size_t format_name_len_in_char = ::UTF8StrLenInChar(byte_ptr_cast(format_name.format_name()));
                if (max_format_name_len_in_char < format_name_len_in_char) {
                    max_format_name_len_in_char = format_name_len_in_char;
                }
            }

            if (all_format_names_are_ASCII_strings &&
                (max_format_name_len_in_char >= (32 /* formatName(32) */ / sizeof(uint16_t)))) {
                return true;
            }
        }

        return false;
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = 0;

        size_t result = ::snprintf(buffer + length, size - length,
            "FormatListPDU:");
        length += ((result < size - length) ? result : (size - length - 1));

        for (auto & format_name : this->format_names) {
            size_t result = ::snprintf(buffer + length, size - length, " ");
            length += ((result < size - length) ? result : (size - length - 1));

            result = format_name.str(buffer + length, size - length);
            length += ((result < size - length) ? result : (size - length - 1));
        }

        return length;
    }

public:
    void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, "%s", buffer);
    }
};  // FormatListPDUEx

inline static bool FormatListPDUEx_contains_data_in_format(const FormatListPDUEx & format_list_pdu, uint32_t formatId) {
    for (size_t i = 0, c = format_list_pdu.num_format_names(); i < c; ++i) {
        FormatName const & format_name = format_list_pdu.format_name(i);
        if (format_name.formatId() == formatId) {
            return true;
        }
    }

    return false;
}

} // namespace RDPECLIP
