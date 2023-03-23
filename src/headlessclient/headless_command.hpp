/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "utils/sugar/array_view.hpp"

#include <string>
#include <chrono>


class RdpInput;

struct HeadlessCommand
{
    enum class ErrorType : unsigned
    {
        TooManyArgument,
        MissingArgument,
        InvalidFormat,
        UnknownCommand,
    };

    static char const* error_to_cstring(ErrorType error_type)
    {
        switch (error_type) {
            case ErrorType::TooManyArgument: return "Too many argument";
            case ErrorType::MissingArgument: return "Missing argument";
            case ErrorType::InvalidFormat: return "Invalid argument";
            case ErrorType::UnknownCommand: return "Unknown command";
        }
        return "Unknown error";
    }

    uint16_t mouse_x = 0;
    uint16_t mouse_y = 0;

    uint16_t screen_width = 800;
    uint16_t screen_height = 600;

    bool is_kbdmap_en = true;
    bool is_rdp = true;
    bool enable_wrm = false;
    bool enable_png = false;
    bool enable_record_transport = false;
    unsigned port = 3389;

    ErrorType error_type;
    unsigned index_param_error;

    // parameter value with Result::Fail
    // message with Result::OutputResult (for help command)
    chars_view output_message;

    std::string session_id;
    std::string png_path;
    std::string wrm_path;
    std::string record_transport_path;
    std::string ip_address;
    std::string username;
    std::string password;

    // Sleep, RepetitionCommand, KeyDelay, MouseDelay
    std::chrono::milliseconds delay {-1};

    int repeat_delay = -1;

    enum class [[nodiscard]] Result
    {
        Fail,
        Ok,
        OutputResult,

        Connect,
        Reconnect,
        Disconnect,
        PrintScreen,
        ConfigFile,
        ConfigStr,
        RepetitionCommand,
        Sleep,
        KeyDelay,
        MouseDelay,
        Quit,
    };

    // format:
    //
    // cmd := ('scancode'|'sc') (ws (mod '+')* value_and_flag)*
    //      | ('mouse'|'m') (ws ('+'|'-')? value_and_flag)*
    //      | ('unicode'|'uc') ( ws integer (',' up_down_flag?)? )*
    //      | ('key'|'k') ws key*
    //      | ('text'|'t' | 'textln'|'tln') ws .*
    //      | ('move'|'mv') ws integer ws integer
    //      | ('scroll' | 'hscroll') ws '-'?decimal
    //      | 'lock' (ws value)*
    //      | 'kbd' ws ('fr'|'en')
    //      | ('help'|'h'|'?') (ws name (ws name)?)?
    //      | ('connect'|'co'|'rdp'|'vnc') (ws addr port?)?
    //      | ('disconnect'|'disco' | 'reconnect'|'reco')
    //      | ('wrm' | 'record-transport') ws bool (ws filename)?
    //      | ('png' | 'p') (ws filename)?
    //      | ('basename' | 'sid') (ws filename)?
    //      | ('configfile' | 'conff' | 'f') ws filename
    //      | ('configstr' | 'conf') ws str
    //      | 'repeat' ws decimal decimal? cmd
    //      | 'sleep' delay*
    //      | 'keydelay' delay*
    //      | 'mousedelay' delay*
    //      | ('quit' | 'q')
    //      | '#' .*
    // ws := ' '
    // delay := (decimal ('/' decimal) | decimal? '.' decimal) ('ms'|'s'|'m'|'min')?
    // mod := 'R'? 'S' 'hift'?
    //      | 'R'? 'C' 'trl'?
    //      | 'R'? 'M' 'eta'?
    //      | 'R'? 'W' 'in'?
    //      | 'R'? 'A' 'lt'?
    //      | 'G' | 'AltGr'
    // value := hex | name
    // key := key_mod? ( key_named | . )
    // key_mod := '!' | '+' | '^' | '#' | '~'
    // key_named := '{' value (' ' ('up' | 'down' | decimal))? '}'
    // integer := hex | decimal
    // value_and_flag = value (',' up_down_flag?)?
    // up_down_flag := '0'|'up' | '1'|'down' | '2'|'downup'
    // bool := '0'|'off' | '1'|'on'
    Result execute_command(chars_view cmd, RdpInput& mod);

    chars_view help_all() const;
    chars_view help(chars_view cmd) const;
};
