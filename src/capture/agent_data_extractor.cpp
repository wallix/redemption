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

#include "core/window_constants.hpp"
#include "capture/agent_data_extractor.hpp"
#include "core/log_id.hpp"
#include "utils/log.hpp"
#include "utils/utf.hpp"

#include <algorithm>
#include <utility>
#include <string_view>

using std::begin;
using std::end;

namespace
{
    using Av = chars_view;

    using Pair = std::pair<std::string_view, LogId>;

    auto pair_comparator()
    {
        return [](auto& pair1, auto& pair2){
            auto a = pair1.first;
            auto b = pair2.first;
            return a.size() < b.size()
                || (a.size() == b.size()
                 && std::lexicographical_compare(begin(a), end(a), begin(b), end(b)));
        };
    }

    // TODO constexpr with C++20
    // TODO sould be a hash map
    inline auto const sorted_log_id_strings = []() noexcept {
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_EMSCRIPTEN_IGNORE("-Wmissing-variable-declarations")
        std::array pairs{
            #define f(x, cat) Pair(#x "", LogId::x),
            X_LOG_ID(f)
            #undef f
        };
        REDEMPTION_DIAGNOSTIC_POP

        std::sort(begin(pairs), end(pairs), pair_comparator());

        return pairs;
    }();

    // TODO constexpr with C++20
    inline auto const sorted_old_log_names = []() noexcept {
        using namespace std::string_view_literals;
        std::array pairs{
            "EndingInProgress"sv,
            "PasswordTextBox.SetFocus"sv,
            "ConsentUI.IsVisible"sv,
            "InputLanguage"sv,
            "NewProcess"sv,
            "CompletedProcess"sv,
            "OutboundConnectionBlocked"sv,
            "ForegroundWindowChanged"sv,
            "Button.Clicked"sv,
            "Edit.Changed"sv,
        };

        std::sort(begin(pairs), end(pairs));

        return pairs;
    }();
} // anonymous namespace

bool AgentDataExtractor::extract_old_format_list(Av data)
{
    auto find = [](Av const & s, char c) {
        auto p = std::find(begin(s), end(s), c);
        return p == end(s) ? nullptr : p;
    };

    auto const pos_separator = find(data, '=');

    auto kv_list = [&](auto ...kv){
        auto* p = this->kvlogs.data();
        ((*p++ = kv), ...);
        this->kvlogs_end = p;
        return true;
    };


    if (pos_separator) {
        auto ileft = [](Av s, char const * pos) { return Av(begin(s), pos - begin(s)); };
        auto right = [](Av s, char const * pos) { return Av(pos + 1, begin(s) + s.size() - (pos + 1)); };

        auto const order = ileft(data, pos_separator);
        auto const parameters = (data.back() == '\x0')
          ? Av(pos_separator+1, data.end()-1)
          : Av(pos_separator+1, data.end());

        auto line_with_1_var = [&](Av var1) {
            return kv_list(KVLog(var1, parameters));
        };
        auto line_with_2_var = [&](Av var1, Av var2) {
            if (auto const subitem_separator = find(parameters, '\x01')) {
                return kv_list(
                    KVLog(var1, ileft(parameters, subitem_separator)),
                    KVLog(var2, right(parameters, subitem_separator))
                );
            }
            return false;
        };
        auto line_with_3_var = [&](Av var1, Av var2, Av var3) {
            if (auto const subitem_separator = find(parameters, '\x01')) {
                auto const text = ileft(parameters, subitem_separator);
                auto const remaining = right(parameters, subitem_separator);
                if (auto const subitem_separator2 = find(remaining, '\x01')) {
                    return kv_list(
                        KVLog(var1, text),
                        KVLog(var2, ileft(remaining, subitem_separator2)),
                        KVLog(var3, right(remaining, subitem_separator2))
                    );
                }
            }
            return false;
        };
        auto line_with_4_var = [&](Av var1, Av var2, Av var3, Av var4) {
            if (auto const subitem_separator = find(parameters, '\x01')) {
                auto const text = ileft(parameters, subitem_separator);
                auto const remaining = right(parameters, subitem_separator);
                if (auto const subitem_separator2 = find(remaining, '\x01')) {
                    auto const text2 = ileft(remaining, subitem_separator2);
                    auto const remaining2 = right(remaining, subitem_separator2);
                    if (auto const subitem_separator3 = find(remaining2, '\x01')) {
                        auto const text3 = ileft(remaining2, subitem_separator3);
                        auto const text4 = right(remaining2, subitem_separator3);
                        return kv_list(
                            KVLog(var1, text),
                            KVLog(var2, text2),
                            KVLog(var3, text3),
                            KVLog(var4, text4)
                        );
                    }
                }
            }
            return false;
        };
        auto line_with_5_var = [&](Av var1, Av var2, Av var3, Av var4, Av var5) {
            if (auto const subitem_separator = find(parameters, '\x01')) {
                auto const text = ileft(parameters, subitem_separator);
                auto const remaining = right(parameters, subitem_separator);
                if (auto const subitem_separator2 = find(remaining, '\x01')) {
                    auto const text2 = ileft(remaining, subitem_separator2);
                    auto const remaining2 = right(remaining, subitem_separator2);
                    if (auto const subitem_separator3 = find(remaining2, '\x01')) {
                        auto const text3 = ileft(remaining2, subitem_separator3);
                        auto const remaining3 = right(remaining2, subitem_separator3);
                        if (auto const subitem_separator4 = find(remaining3, '\x01')) {
                            auto const text4 = ileft(remaining3, subitem_separator4);
                            auto const text5 = right(remaining3, subitem_separator4);
                            return kv_list(
                                KVLog(var1, text),
                                KVLog(var2, text2),
                                KVLog(var3, text3),
                                KVLog(var4, text4),
                                KVLog(var5, text5)
                            );
                        }
                    }
                }
            }
            return false;
        };
        auto line_with_6_var = [&](Av var1, Av var2, Av var3, Av var4, Av var5, Av var6) {
            if (auto const subitem_separator = find(parameters, '\x01')) {
                auto const text = ileft(parameters, subitem_separator);
                auto const remaining = right(parameters, subitem_separator);
                if (auto const subitem_separator2 = find(remaining, '\x01')) {
                    auto const text2 = ileft(remaining, subitem_separator2);
                    auto const remaining2 = right(remaining, subitem_separator2);
                    if (auto const subitem_separator3 = find(remaining2, '\x01')) {
                        auto const text3 = ileft(remaining2, subitem_separator3);
                        auto const remaining3 = right(remaining2, subitem_separator3);
                        if (auto const subitem_separator4 = find(remaining3, '\x01')) {
                            auto const text4 = ileft(remaining3, subitem_separator4);
                            auto const remaining4 = right(remaining3, subitem_separator4);
                            if (auto const subitem_separator5 = find(remaining4, '\x01')) {
                                auto const text5 = ileft(remaining4, subitem_separator5);
                                auto const text6 = right(remaining4, subitem_separator5);
                                return kv_list(
                                    KVLog(var1, text),
                                    KVLog(var2, text2),
                                    KVLog(var3, text3),
                                    KVLog(var4, text4),
                                    KVLog(var5, text5),
                                    KVLog(var6, text6)
                                );
                            }
                        }
                    }
                }
            }
            return false;
        };
        auto line_with_7_var = [&](Av var1, Av var2, Av var3, Av var4, Av var5, Av var6, Av var7) {
            if (auto const subitem_separator = find(parameters, '\x01')) {
                auto const text = ileft(parameters, subitem_separator);
                auto const remaining = right(parameters, subitem_separator);
                if (auto const subitem_separator2 = find(remaining, '\x01')) {
                    auto const text2 = ileft(remaining, subitem_separator2);
                    auto const remaining2 = right(remaining, subitem_separator2);
                    if (auto const subitem_separator3 = find(remaining2, '\x01')) {
                        auto const text3 = ileft(remaining2, subitem_separator3);
                        auto const remaining3 = right(remaining2, subitem_separator3);
                        if (auto const subitem_separator4 = find(remaining3, '\x01')) {
                            auto const text4 = ileft(remaining3, subitem_separator4);
                            auto const remaining4 = right(remaining3, subitem_separator4);
                            if (auto const subitem_separator5 = find(remaining4, '\x01')) {
                                auto const text5 = ileft(remaining4, subitem_separator5);
                                auto const remaining5 = right(remaining4, subitem_separator5);
                                if (auto const subitem_separator6 = find(remaining5, '\x01')) {
                                    auto const text6 = ileft(remaining5, subitem_separator6);
                                    auto const text7 = right(remaining5, subitem_separator6);
                                    return kv_list(
                                        KVLog(var1, text),
                                        KVLog(var2, text2),
                                        KVLog(var3, text3),
                                        KVLog(var4, text4),
                                        KVLog(var5, text5),
                                        KVLog(var6, text6),
                                        KVLog(var7, text7)
                                    );
                                }
                            }
                        }
                    }
                }
            }
            return false;
        };

        std::string_view order_sv{order.data(), order.size()};
        auto it = std::lower_bound(begin(sorted_log_id_strings), end(sorted_log_id_strings),
            Pair{order_sv, LogId()}, pair_comparator());

        if (it == end(sorted_log_id_strings) || it->first != order_sv) {
            if (std::lower_bound(begin(sorted_old_log_names), end(sorted_old_log_names), order_sv)
                == end(sorted_old_log_names)
            ) {
                LOG(LOG_WARNING,
                    "MetaDataExtractor(): Invalid data format. Data=\"%.*s\"",
                    int(data.size()), data.data());
            }
            return false;
        }

        this->id = it->second;

        switch (this->id) {
            case LogId::PASSWORD_TEXT_BOX_GET_FOCUS:
            case LogId::UAC_PROMPT_BECOME_VISIBLE:
            case LogId::SESSION_LOCKED:
            case LogId::UNIDENTIFIED_INPUT_FIELD_GET_FOCUS:
                return line_with_1_var("status"_av);
            case LogId::INPUT_LANGUAGE:
                return line_with_2_var("identifier"_av, "display_name"_av);
            case LogId::NEW_PROCESS:
            case LogId::COMPLETED_PROCESS:
                return line_with_1_var("command_line"_av);
            case LogId::OUTBOUND_CONNECTION_BLOCKED:
            case LogId::OUTBOUND_CONNECTION_DETECTED:
                return line_with_2_var("rule"_av, "application_name"_av);
            case LogId::FOREGROUND_WINDOW_CHANGED:
                return line_with_3_var("windows"_av, "class"_av, "command_line"_av);
            case LogId::BUTTON_CLICKED:
                return line_with_2_var("windows"_av, "button"_av);
            case LogId::CHECKBOX_CLICKED:
                if (auto const subitem_separator = find(parameters, '\x01')) {
                    auto const text = ileft(parameters, subitem_separator);
                    auto const remaining = right(parameters, subitem_separator);
                    if (auto const subitem_separator2 = find(remaining, '\x01')) {
                        auto const r = right(remaining, subitem_separator2);
                        return kv_list(
                            KVLog("windows"_av,  text),
                            KVLog("checkbox"_av, ileft(remaining, subitem_separator2)),
                            KVLog("state"_av,    ::button_state_to_string('0'-r[0]))
                        );
                    }
                }
                break;
            case LogId::EDIT_CHANGED:
                return line_with_2_var("windows"_av, "edit"_av);
            case LogId::DRIVE_REDIRECTION_USE:
                return line_with_2_var("device_name"_av, "device_type"_av);
            case LogId::DRIVE_REDIRECTION_READ:
            case LogId::DRIVE_REDIRECTION_WRITE:
            case LogId::DRIVE_REDIRECTION_DELETE:
                return line_with_1_var("file_name"_av);
            case LogId::DRIVE_REDIRECTION_READ_EX:
            case LogId::DRIVE_REDIRECTION_WRITE_EX:
                return line_with_3_var("file_name"_av, "size"_av, "sha256"_av);
            case LogId::DRIVE_REDIRECTION_RENAME:
                return line_with_2_var("old_file_name"_av, "new_file_name"_av);
            case LogId::CB_COPYING_PASTING_FILE_TO_REMOTE_SESSION:
            case LogId::CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION:
                return line_with_3_var("file_name"_av, "size"_av, "sha256"_av);
            case LogId::CLIENT_EXECUTE_REMOTEAPP:
                return line_with_1_var("exe_or_file"_av);
            case LogId::CERTIFICATE_CHECK_SUCCESS:
            case LogId::SERVER_CERTIFICATE_NEW:
            case LogId::SERVER_CERTIFICATE_MATCH_SUCCESS:
            case LogId::SERVER_CERTIFICATE_MATCH_FAILURE:
            case LogId::SERVER_CERTIFICATE_ERROR:
                return line_with_1_var("description"_av);
            case LogId::OUTBOUND_CONNECTION_BLOCKED_2:
            case LogId::OUTBOUND_CONNECTION_DETECTED_2:
                return line_with_5_var("rule"_av, "app_name"_av, "app_cmd_line"_av, "dst_addr"_av, "dst_port"_av);
            case LogId::STARTUP_APPLICATION_FAIL_TO_RUN:
                return line_with_2_var("application_name"_av, "raw_result"_av);
            case LogId::STARTUP_APPLICATION_FAIL_TO_RUN_2:
                return line_with_3_var("application_name"_av, "raw_result"_av, "raw_result_message"_av);
            case LogId::PROCESS_BLOCKED:
            case LogId::PROCESS_DETECTED:
                return line_with_3_var("rule"_av, "app_name"_av, "app_cmd_line"_av);
            case LogId::KERBEROS_TICKET_CREATION:
            case LogId::KERBEROS_TICKET_DELETION:
                return line_with_7_var("encryption_type"_av, "client_name"_av, "server_name"_av, "start_time"_av, "end_time"_av, "renew_time"_av, "flags"_av);
            case LogId::CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION:
            case LogId::CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION:
                return line_with_2_var("format"_av, "size"_av);
            case LogId::CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION_EX:
            case LogId::CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION_EX:
                if (auto const subitem_separator = find(parameters, '\x01')) {
                    auto const format = ileft(parameters, subitem_separator);
                    auto const remaining = right(parameters, subitem_separator);
                    if (auto const subitem_separator2 = find(remaining, '\x01')) {
                        Av partial_data = right(remaining, subitem_separator2);

                        char const * tmp_d = partial_data.data();
                        size_t tmp_l       = partial_data.size();

                        // extract valid utf8 string
                        while (tmp_l) {
                            size_t nbb = ::UTF8CharNbBytes(::byte_ptr_cast(tmp_d));
                            if (nbb > tmp_l) {
                                partial_data = partial_data.first(partial_data.size() - tmp_l);
                                break;
                            }

                            tmp_l -= nbb;
                            tmp_d += nbb;
                        }

                        return kv_list(
                            KVLog("format"_av,  format),
                            KVLog("size"_av, ileft(remaining, subitem_separator2)),
                            KVLog("partial_data"_av, partial_data)
                        );
                    }
                }
                break;

            case LogId::WEB_ATTEMPT_TO_PRINT:
            case LogId::WEB_DOCUMENT_COMPLETE:
                return line_with_2_var("url"_av, "title"_av);
            case LogId::WEB_BEFORE_NAVIGATE:
                return line_with_2_var("url"_av, "post"_av);
            case LogId::WEB_NAVIGATE_ERROR:
                return line_with_4_var("url"_av, "title"_av, "code"_av, "display_name"_av);
            case LogId::WEB_NAVIGATION:
            case LogId::WEB_THIRD_PARTY_URL_BLOCKED:
                return line_with_1_var("url"_av);
            case LogId::WEB_PRIVACY_IMPACTED:
                return line_with_1_var("impacted"_av);
            case LogId::WEB_ENCRYPTION_LEVEL_CHANGED:
                return line_with_2_var("identifier"_av, "display_name"_av);
            case LogId::FILE_VERIFICATION:
                return line_with_3_var("direction"_av, "file_name"_av, "status"_av);
            case LogId::FILE_VERIFICATION_ERROR:
                return line_with_2_var("icap_service"_av, "status"_av);
            case LogId::GROUP_MEMBERSHIP:
                return line_with_1_var("groups"_av);
            case LogId::PROBE_STATUS:
                return line_with_1_var("status"_av);
            case LogId::KILL_PATTERN_DETECTED:
            case LogId::NOTIFY_PATTERN_DETECTED:
                return line_with_1_var("pattern"_av);
            case LogId::SESSION_DISCONNECTION:
                return line_with_1_var("duration"_av);
            // extracted by OCR / FOREGROUND_WINDOW_CHANGED
            case LogId::TITLE_BAR:
            // re-extracted
            case LogId::KBD_INPUT:
            // useless (?)
            case LogId::CONNECTION_FAILED:
            case LogId::SESSION_CREATION_FAILED:
            case LogId::SESSION_ENDING_IN_PROGRESS:
            case LogId::SESSION_ESTABLISHED_SUCCESSFULLY:
                return false;
            // unknown old id (or invalid id in old format)
            case LogId::ACCOUNT_MANIPULATION_BLOCKED:
            case LogId::ACCOUNT_MANIPULATION_DETECTED:
                return line_with_6_var("operation"_av, "server_name"_av, "group_name"_av, "account_name"_av, "app_name"_av, "app_cmd_line"_av);

            case LogId::DYNAMIC_CHANNEL_CREATION_ALLOWED:
            case LogId::DYNAMIC_CHANNEL_CREATION_REJECTED:
                return line_with_1_var("channel_name"_av);

            default:
                LOG(LOG_WARNING,
                    "MetaDataExtractor(): Unexpected order. Data=\"%.*s\"",
                    int(data.size()), data.data());
        }
    }

    return false;
}

bool AgentDataExtractor::is_relevant_log_id(LogId id) noexcept
{
    switch (id) {
        // extracted by OCR / FOREGROUND_WINDOW_CHANGED
        case LogId::TITLE_BAR:
        // re-extracted
        case LogId::KBD_INPUT:
        // useless (?)
        case LogId::CONNECTION_FAILED:
        case LogId::SESSION_CREATION_FAILED:
        case LogId::SESSION_ENDING_IN_PROGRESS:
            return false;
        case LogId::SESSION_ESTABLISHED_SUCCESSFULLY:
        case LogId::PASSWORD_TEXT_BOX_GET_FOCUS:
        case LogId::UAC_PROMPT_BECOME_VISIBLE:
        case LogId::SESSION_LOCKED:
        case LogId::UNIDENTIFIED_INPUT_FIELD_GET_FOCUS:
        case LogId::INPUT_LANGUAGE:
        case LogId::NEW_PROCESS:
        case LogId::COMPLETED_PROCESS:
        case LogId::OUTBOUND_CONNECTION_BLOCKED:
        case LogId::OUTBOUND_CONNECTION_DETECTED:
        case LogId::FOREGROUND_WINDOW_CHANGED:
        case LogId::BUTTON_CLICKED:
        case LogId::CHECKBOX_CLICKED:
        case LogId::EDIT_CHANGED:
        case LogId::DRIVE_REDIRECTION_USE:
        case LogId::DRIVE_REDIRECTION_READ:
        case LogId::DRIVE_REDIRECTION_WRITE:
        case LogId::DRIVE_REDIRECTION_DELETE:
        case LogId::DRIVE_REDIRECTION_READ_EX:
        case LogId::DRIVE_REDIRECTION_WRITE_EX:
        case LogId::DRIVE_REDIRECTION_RENAME:
        case LogId::CB_COPYING_PASTING_FILE_TO_REMOTE_SESSION:
        case LogId::CB_COPYING_PASTING_FILE_FROM_REMOTE_SESSION:
        case LogId::CLIENT_EXECUTE_REMOTEAPP:
        case LogId::CERTIFICATE_CHECK_SUCCESS:
        case LogId::SERVER_CERTIFICATE_NEW:
        case LogId::SERVER_CERTIFICATE_MATCH_SUCCESS:
        case LogId::SERVER_CERTIFICATE_MATCH_FAILURE:
        case LogId::SERVER_CERTIFICATE_ERROR:
        case LogId::OUTBOUND_CONNECTION_BLOCKED_2:
        case LogId::OUTBOUND_CONNECTION_DETECTED_2:
        case LogId::STARTUP_APPLICATION_FAIL_TO_RUN:
        case LogId::STARTUP_APPLICATION_FAIL_TO_RUN_2:
        case LogId::PROCESS_BLOCKED:
        case LogId::PROCESS_DETECTED:
        case LogId::KERBEROS_TICKET_CREATION:
        case LogId::KERBEROS_TICKET_DELETION:
        case LogId::CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION:
        case LogId::CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION:
        case LogId::CB_COPYING_PASTING_DATA_TO_REMOTE_SESSION_EX:
        case LogId::CB_COPYING_PASTING_DATA_FROM_REMOTE_SESSION_EX:
        case LogId::WEB_ATTEMPT_TO_PRINT:
        case LogId::WEB_DOCUMENT_COMPLETE:
        case LogId::WEB_BEFORE_NAVIGATE:
        case LogId::WEB_NAVIGATE_ERROR:
        case LogId::WEB_NAVIGATION:
        case LogId::WEB_THIRD_PARTY_URL_BLOCKED:
        case LogId::WEB_PRIVACY_IMPACTED:
        case LogId::WEB_ENCRYPTION_LEVEL_CHANGED:
        case LogId::FILE_VERIFICATION:
        case LogId::FILE_VERIFICATION_ERROR:
        case LogId::GROUP_MEMBERSHIP:
        case LogId::PROBE_STATUS:
        case LogId::KILL_PATTERN_DETECTED:
        case LogId::NOTIFY_PATTERN_DETECTED:
        case LogId::SESSION_DISCONNECTION:
        case LogId::TEXT_VERIFICATION:
        case LogId::ACCOUNT_MANIPULATION_BLOCKED:
        case LogId::ACCOUNT_MANIPULATION_DETECTED:
        case LogId::BESTSAFE_SERVICE_LOG:
        case LogId::DYNAMIC_CHANNEL_CREATION_ALLOWED:
        case LogId::DYNAMIC_CHANNEL_CREATION_REJECTED:
        case LogId::FILE_BLOCKED:
            ;
    }
    return true;
}
