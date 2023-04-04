/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/mod/accumulate_input_mod.hpp"

#include "headlessclient/headless_command.hpp"
#include "utils/strutils.hpp"
#include "utils/sugar/int_to_chars.hpp"


RED_AUTO_TEST_CASE(TestExecuteCommand)
{
    auto cmd_err_to_string = [](HeadlessCommand cmd) {
        chars_view type;

        #define CASE(name) case HeadlessCommand::ErrorType::name: type = #name ""_av; break
        switch (cmd.error_type) {
            CASE(TooManyArgument);
            CASE(MissingArgument);
            CASE(InvalidFormat);
            CASE(UnknownCommand);
        }
        #undef CASE

        return str_concat(
            "index_param="_av, int_to_decimal_chars(cmd.index_param_error), ' ',
            "param='"_av, cmd.output_message, "' "_av,
            "expected='"_av, cmd.expected_arg, "' "_av,
            "type="_av, type
        );
    };

    using CmdResult = HeadlessCommand::Result;

    HeadlessCommand cmd_ctx;
    AccumulateInputMod mod;
    CmdResult r;

    #define check_cmd_ex(cmd, err_msg, msg, cmd_result) do { RED_TEST_CONTEXT("CMD: `" cmd "`") { \
        auto expected_result = cmd_result;                                                        \
        RED_CHECK(expected_result == (r = cmd_ctx.execute_command(cmd ""_av, mod)));              \
        if (r == CmdResult::Fail) RED_CHECK(cmd_err_to_string(cmd_ctx) == err_msg ""_av);         \
        RED_CHECK(mod.events() == msg ""_av);                                                     \
    } } while (0)

    #define check_cmd(cmd, err_msg, msg)  \
        check_cmd_ex(cmd, err_msg, msg, *err_msg ? CmdResult::Fail : CmdResult::Ok)


    check_cmd("", "", "");
    check_cmd("xx", "index_param=0 param='xx' expected='command' type=UnknownCommand", "");
    check_cmd(" xx", "index_param=0 param='xx' expected='command' type=UnknownCommand", "");


    // scancode
    //@{
    check_cmd("sc qq", "index_param=1 param='qq' expected='scancode' type=InvalidFormat", "");

    check_cmd("sc q", "",
        "{KbdFlags=0x0000, Scancode=0x10}, "
        "{KbdFlags=0x8000, Scancode=0x10}");

    check_cmd("sc Q", "",
        "{KbdFlags=0x0000, Scancode=0x2A}, "
        "{KbdFlags=0x0000, Scancode=0x10}, "
        "{KbdFlags=0x8000, Scancode=0x10}, "
        "{KbdFlags=0x8000, Scancode=0x2A}");

    check_cmd("sc S+Q", "",
        "{KbdFlags=0x0000, Scancode=0x2A}, "
        "{KbdFlags=0x0000, Scancode=0x10}, "
        "{KbdFlags=0x8000, Scancode=0x10}, "
        "{KbdFlags=0x8000, Scancode=0x2A}");

    check_cmd("sc S+q", "",
        "{KbdFlags=0x0000, Scancode=0x2A}, "
        "{KbdFlags=0x0000, Scancode=0x10}, "
        "{KbdFlags=0x8000, Scancode=0x10}, "
        "{KbdFlags=0x8000, Scancode=0x2A}");

    check_cmd("sc Shift+Shift+Shift+q", "",
        "{KbdFlags=0x0000, Scancode=0x2A}, "
        "{KbdFlags=0x0000, Scancode=0x10}, "
        "{KbdFlags=0x8000, Scancode=0x10}, "
        "{KbdFlags=0x8000, Scancode=0x2A}");

    check_cmd("sc q q w", "",
        "{KbdFlags=0x0000, Scancode=0x10}, "
        "{KbdFlags=0x8000, Scancode=0x10}, "
        "{KbdFlags=0x0000, Scancode=0x10}, "
        "{KbdFlags=0x8000, Scancode=0x10}, "
        "{KbdFlags=0x0000, Scancode=0x11}, "
        "{KbdFlags=0x8000, Scancode=0x11}");

    check_cmd("sc 0x10 0x10 0x11", "",
        "{KbdFlags=0x0000, Scancode=0x10}, "
        "{KbdFlags=0x8000, Scancode=0x10}, "
        "{KbdFlags=0x0000, Scancode=0x10}, "
        "{KbdFlags=0x8000, Scancode=0x10}, "
        "{KbdFlags=0x0000, Scancode=0x11}, "
        "{KbdFlags=0x8000, Scancode=0x11}");

    check_cmd("sc q,1 w, 0x10,0", "",
        "{KbdFlags=0x0000, Scancode=0x10}, "
        "{KbdFlags=0x0000, Scancode=0x11}, "
        "{KbdFlags=0x8000, Scancode=0x11}, "
        "{KbdFlags=0x8000, Scancode=0x10}");

    check_cmd("sc pause", "",
        "{KbdFlags=0x0200, Scancode=0x1D}, {KbdFlags=0x0000, Scancode=0x45}, "
        "{KbdFlags=0x8200, Scancode=0x1D}, {KbdFlags=0x8000, Scancode=0x45}");

    check_cmd("sc ,",   "", "{KbdFlags=0x0000, Scancode=0x33}, {KbdFlags=0x8000, Scancode=0x33}");
    check_cmd("sc ,,",  "", "{KbdFlags=0x0000, Scancode=0x33}, {KbdFlags=0x8000, Scancode=0x33}");
    check_cmd("sc ,,1", "", "{KbdFlags=0x0000, Scancode=0x33}");
    //@}


    // key
    //@{
    check_cmd("key", "", "");

    check_cmd("key q", "",
        "{KbdFlags=0x0000, Scancode=0x10}, "
        "{KbdFlags=0x8000, Scancode=0x10}");

    check_cmd("key qq", "",
        "{KbdFlags=0x0000, Scancode=0x10}, "
        "{KbdFlags=0x8000, Scancode=0x10}, "
        "{KbdFlags=0x0000, Scancode=0x10}, "
        "{KbdFlags=0x8000, Scancode=0x10}");

    check_cmd("key {q", "index_param=0 param='{q' expected='key' type=InvalidFormat", "");

    check_cmd("key abcA B", "",
        // a
        "{KbdFlags=0x0000, Scancode=0x1E}, "
        "{KbdFlags=0x8000, Scancode=0x1E}, "
        // b
        "{KbdFlags=0x0000, Scancode=0x30}, "
        "{KbdFlags=0x8000, Scancode=0x30}, "
        // c
        "{KbdFlags=0x0000, Scancode=0x2E}, "
        "{KbdFlags=0x8000, Scancode=0x2E}, "
        // A = shift+a
        "{KbdFlags=0x0000, Scancode=0x2A}, "
        "{KbdFlags=0x0000, Scancode=0x1E}, "
        "{KbdFlags=0x8000, Scancode=0x1E}, "
        "{KbdFlags=0x8000, Scancode=0x2A}, "
        // space
        "{KbdFlags=0x0000, Scancode=0x39}, "
        "{KbdFlags=0x8000, Scancode=0x39}, "
        // B= shift+b
        "{KbdFlags=0x0000, Scancode=0x2A}, "
        "{KbdFlags=0x0000, Scancode=0x30}, "
        "{KbdFlags=0x8000, Scancode=0x30}, "
        "{KbdFlags=0x8000, Scancode=0x2A}");

    check_cmd("key ^a#b!!#~cd+A", "",
        // ctrl+a
        "{KbdFlags=0x0000, Scancode=0x1D}, "
        "{KbdFlags=0x0000, Scancode=0x1E}, "
        "{KbdFlags=0x8000, Scancode=0x1E}, "
        "{KbdFlags=0x8000, Scancode=0x1D}, "
        // ctrl+b
        "{KbdFlags=0x0100, Scancode=0x5B}, "
        "{KbdFlags=0x0000, Scancode=0x30}, "
        "{KbdFlags=0x8000, Scancode=0x30}, "
        "{KbdFlags=0x8100, Scancode=0x5B}, "
        // alt+meta+altgr+c
        "{KbdFlags=0x0000, Scancode=0x38}, "
        "{KbdFlags=0x0100, Scancode=0x5B}, "
        "{KbdFlags=0x0100, Scancode=0x38}, "
        "{KbdFlags=0x0000, Scancode=0x2E}, "
        "{KbdFlags=0x8000, Scancode=0x2E}, "
        "{KbdFlags=0x8100, Scancode=0x38}, "
        "{KbdFlags=0x8100, Scancode=0x5B}, "
        "{KbdFlags=0x8000, Scancode=0x38}, "
        // d
        "{KbdFlags=0x0000, Scancode=0x20}, "
        "{KbdFlags=0x8000, Scancode=0x20}, "
        // +A = Shift+a
        "{KbdFlags=0x0000, Scancode=0x2A}, "
        "{KbdFlags=0x0000, Scancode=0x1E}, "
        "{KbdFlags=0x8000, Scancode=0x1E}, "
        "{KbdFlags=0x8000, Scancode=0x2A}");

    check_cmd("key {q 3}{a}{q down}{q up}", "",
        // q * 3
        "{KbdFlags=0x0000, Scancode=0x10}, "
        "{KbdFlags=0x0000, Scancode=0x10}, "
        "{KbdFlags=0x0000, Scancode=0x10}, "
        "{KbdFlags=0x8000, Scancode=0x10}, "
        // a
        "{KbdFlags=0x0000, Scancode=0x1E}, "
        "{KbdFlags=0x8000, Scancode=0x1E}, "
        // q down
        "{KbdFlags=0x0000, Scancode=0x10}, "
        // q up
        "{KbdFlags=0x8000, Scancode=0x10}");
    //@}


    // kbd
    //@{
    check_cmd_ex("kbd fr", "", "", CmdResult::KbdChange);
    check_cmd("sc q", "",
        "{KbdFlags=0x0000, Scancode=0x1E}, "
        "{KbdFlags=0x8000, Scancode=0x1E}");
    check_cmd("k q", "",
        "{KbdFlags=0x0000, Scancode=0x1E}, "
        "{KbdFlags=0x8000, Scancode=0x1E}");

    check_cmd_ex("kbd en", "", "", CmdResult::KbdChange);
    check_cmd("sc q", "",
        "{KbdFlags=0x0000, Scancode=0x10}, "
        "{KbdFlags=0x8000, Scancode=0x10}");
    check_cmd("k q", "",
        "{KbdFlags=0x0000, Scancode=0x10}, "
        "{KbdFlags=0x8000, Scancode=0x10}");
    //@}


    // unicode (🚀 = 0x1F680)
    //@{
    check_cmd("uc 0x1F680,1", "",
        "{KbdFlags=0x0000, Unicode=0xD83D}, {KbdFlags=0x0000, Unicode=0xDE80}");
    check_cmd("uc 0x1F680,0", "",
        "{KbdFlags=0x8000, Unicode=0xD83D}, {KbdFlags=0x8000, Unicode=0xDE80}");
    check_cmd("uc 0x1F680", "",
        "{KbdFlags=0x0000, Unicode=0xD83D}, {KbdFlags=0x0000, Unicode=0xDE80}, "
        "{KbdFlags=0x8000, Unicode=0xD83D}, {KbdFlags=0x8000, Unicode=0xDE80}");
    check_cmd("uc 0x1F680,1 0x1F680,0", "",
        "{KbdFlags=0x0000, Unicode=0xD83D}, {KbdFlags=0x0000, Unicode=0xDE80}, "
        "{KbdFlags=0x8000, Unicode=0xD83D}, {KbdFlags=0x8000, Unicode=0xDE80}");
    check_cmd("uc 42", "",
        "{KbdFlags=0x0000, Unicode=0x002A}, {KbdFlags=0x8000, Unicode=0x002A}");
    check_cmd("uc a", "index_param=1 param='a' expected='unicode char or hexadecimal code' type=InvalidFormat", "");
    //@}


    // text
    //@{
    check_cmd("t", "", "");
    check_cmd("t  a", "",
        // space
        "{KbdFlags=0x0000, Unicode=0x0020}, "
        "{KbdFlags=0x8000, Unicode=0x0020}, "
        // a
        "{KbdFlags=0x0000, Unicode=0x0061}, "
        "{KbdFlags=0x8000, Unicode=0x0061}");
    check_cmd("t ab 🚀c", "",
        // a
        "{KbdFlags=0x0000, Unicode=0x0061}, "
        "{KbdFlags=0x8000, Unicode=0x0061}, "
        // b
        "{KbdFlags=0x0000, Unicode=0x0062}, "
        "{KbdFlags=0x8000, Unicode=0x0062}, "
        // space
        "{KbdFlags=0x0000, Unicode=0x0020}, "
        "{KbdFlags=0x8000, Unicode=0x0020}, "
        // 🚀
        "{KbdFlags=0x0000, Unicode=0xD83D}, "
        "{KbdFlags=0x0000, Unicode=0xDE80}, "
        "{KbdFlags=0x8000, Unicode=0xD83D}, "
        "{KbdFlags=0x8000, Unicode=0xDE80}, "
        // c
        "{KbdFlags=0x0000, Unicode=0x0063}, "
        "{KbdFlags=0x8000, Unicode=0x0063}");
    //@}


    // textln
    //@{
    check_cmd("tln", "",
        // enter
        "{KbdFlags=0x0000, Unicode=0x000A}, "
        "{KbdFlags=0x8000, Unicode=0x000A}");
    check_cmd("tln  a", "",
        // space
        "{KbdFlags=0x0000, Unicode=0x0020}, "
        "{KbdFlags=0x8000, Unicode=0x0020}, "
        // a
        "{KbdFlags=0x0000, Unicode=0x0061}, "
        "{KbdFlags=0x8000, Unicode=0x0061}, "
        // enter
        "{KbdFlags=0x0000, Unicode=0x000A}, "
        "{KbdFlags=0x8000, Unicode=0x000A}");
    check_cmd("tln ab 🚀c", "",
        // a
        "{KbdFlags=0x0000, Unicode=0x0061}, "
        "{KbdFlags=0x8000, Unicode=0x0061}, "
        // b
        "{KbdFlags=0x0000, Unicode=0x0062}, "
        "{KbdFlags=0x8000, Unicode=0x0062}, "
        // space
        "{KbdFlags=0x0000, Unicode=0x0020}, "
        "{KbdFlags=0x8000, Unicode=0x0020}, "
        // 🚀
        "{KbdFlags=0x0000, Unicode=0xD83D}, "
        "{KbdFlags=0x0000, Unicode=0xDE80}, "
        "{KbdFlags=0x8000, Unicode=0xD83D}, "
        "{KbdFlags=0x8000, Unicode=0xDE80}, "
        // c
        "{KbdFlags=0x0000, Unicode=0x0063}, "
        "{KbdFlags=0x8000, Unicode=0x0063}, "
        // enter
        "{KbdFlags=0x0000, Unicode=0x000A}, "
        "{KbdFlags=0x8000, Unicode=0x000A}");
    //@}


    // mouse
    //@{
    check_cmd("m b8", "index_param=1 param='b8' expected='mouse flags' type=InvalidFormat", "");
    check_cmd("m b1", "", "{flags=0x9000, x=0, y=0}, {flags=0x1000, x=0, y=0}");
    check_cmd("m b1,1", "", "{flags=0x9000, x=0, y=0}");
    check_cmd("m b1,0", "", "{flags=0x1000, x=0, y=0}");
    check_cmd("m b1 b2", "",
        "{flags=0x9000, x=0, y=0}, {flags=0x1000, x=0, y=0}, "
        "{flags=0xA000, x=0, y=0}, {flags=0x2000, x=0, y=0}");
    //@}


    // move
    //@{
    check_cmd("move 100", "index_param=2 param='missing parameter' expected='y position' type=MissingArgument", "");
    check_cmd("move 110 195", "", "{flags=0x0800, x=110, y=195}");
    check_cmd("move 100 200 300", "index_param=3 param='300' expected='' type=TooManyArgument", "");
    check_cmd("move 100 x", "index_param=2 param='x' expected='y position' type=InvalidFormat", "");

    check_cmd("mv -10 +5", "", "{flags=0x0800, x=100, y=200}");
    //@}


    // scroll
    //@{
    check_cmd("scroll -", "index_param=1 param='-' expected='step' type=InvalidFormat", "");
    check_cmd("scroll 0x", "index_param=1 param='0x' expected='step' type=InvalidFormat", "");
    check_cmd("scroll a", "index_param=1 param='a' expected='step' type=InvalidFormat", "");
    check_cmd("scroll", "",   "{flags=0x0300, x=100, y=200}");
    check_cmd("scroll 1", "", "{flags=0x0300, x=100, y=200}");
    check_cmd("scroll -1", "", "{flags=0x02FF, x=100, y=200}");
    check_cmd("scroll 3", "",
        "{flags=0x0300, x=100, y=200}, "
        "{flags=0x0300, x=100, y=200}, "
        "{flags=0x0300, x=100, y=200}");
    check_cmd("scroll -3", "",
        "{flags=0x02FF, x=100, y=200}, "
        "{flags=0x02FF, x=100, y=200}, "
        "{flags=0x02FF, x=100, y=200}");
    //@}


    // hscroll
    //@{
    check_cmd("hscroll -", "index_param=1 param='-' expected='step' type=InvalidFormat", "");
    check_cmd("hscroll 0x", "index_param=1 param='0x' expected='step' type=InvalidFormat", "");
    check_cmd("hscroll a", "index_param=1 param='a' expected='step' type=InvalidFormat", "");
    check_cmd("hscroll",   "", "{flags=0x0500, x=100, y=200}");
    check_cmd("hscroll 1", "", "{flags=0x0500, x=100, y=200}");
    check_cmd("hscroll -1", "", "{flags=0x04FF, x=100, y=200}");
    check_cmd("hscroll 3", "",
        "{flags=0x0500, x=100, y=200}, "
        "{flags=0x0500, x=100, y=200}, "
        "{flags=0x0500, x=100, y=200}");
    check_cmd("hscroll -3", "",
        "{flags=0x04FF, x=100, y=200}, "
        "{flags=0x04FF, x=100, y=200}, "
        "{flags=0x04FF, x=100, y=200}");
    //@}


    // locks
    //@{
    check_cmd("lock 0", "", "{KeyLocks=0x00}");
    check_cmd("lock 0x02 0x04", "", "{KeyLocks=0x06}");
    check_cmd("lock num scroll", "", "{KeyLocks=0x03}");
    check_cmd("lock num scroll nm kana", "index_param=3 param='nm' expected='lock flags' type=InvalidFormat", "");
    //@}


    // connect
    //@{
    check_cmd_ex("connect ::1",     "", "", CmdResult::Connect);
    RED_CHECK(cmd_ctx.output_message == "::1"_av);
    RED_CHECK(cmd_ctx.port == 3389);
    RED_CHECK(cmd_ctx.is_rdp);
    check_cmd_ex("connect ::2 123", "", "", CmdResult::Connect);
    RED_CHECK(cmd_ctx.output_message == "::2"_av);
    RED_CHECK(cmd_ctx.port == 123);
    RED_CHECK(cmd_ctx.is_rdp);
    check_cmd_ex("connect ::3",     "", "", CmdResult::Connect);
    RED_CHECK(cmd_ctx.output_message == "::3"_av);
    RED_CHECK(cmd_ctx.port == 123);
    RED_CHECK(cmd_ctx.is_rdp);
    check_cmd_ex("connect",     "", "", CmdResult::Connect);
    RED_CHECK(cmd_ctx.output_message == ""_av);
    RED_CHECK(cmd_ctx.port == 123);
    RED_CHECK(cmd_ctx.is_rdp);
    check_cmd("connect ::1 ::2", "index_param=2 param='::2' expected='port' type=InvalidFormat", "");
    check_cmd("connect ::2 123 ::3", "index_param=3 param='::3' expected='' type=TooManyArgument", "");

    check_cmd_ex("rdp ::1",     "", "", CmdResult::Connect);
    RED_CHECK(cmd_ctx.output_message == "::1"_av);
    RED_CHECK(cmd_ctx.port == 3389);
    RED_CHECK(cmd_ctx.is_rdp);
    check_cmd_ex("vnc ::1",     "", "", CmdResult::Connect);
    RED_CHECK(cmd_ctx.output_message == "::1"_av);
    RED_CHECK(cmd_ctx.port == 5900);
    RED_CHECK(!cmd_ctx.is_rdp);
    check_cmd_ex("vnc ::1 5901",     "", "", CmdResult::Connect);
    RED_CHECK(cmd_ctx.output_message == "::1"_av);
    RED_CHECK(cmd_ctx.port == 5901);
    RED_CHECK(!cmd_ctx.is_rdp);
    check_cmd_ex("connect ::1",     "", "", CmdResult::Connect);
    RED_CHECK(cmd_ctx.output_message == "::1"_av);
    RED_CHECK(cmd_ctx.port == 5901);
    RED_CHECK(!cmd_ctx.is_rdp);
    //@}


    // disconnect
    //@{
    check_cmd_ex("disconnect", "", "", CmdResult::Disconnect);
    check_cmd("disconnect abc", "index_param=1 param='abc' expected='' type=TooManyArgument", "");
    //@}


    // wrm
    //@{
    check_cmd_ex("wrm 0", "", "", CmdResult::WrmPath);
    RED_CHECK(!cmd_ctx.output_bool);
    check_cmd_ex("wrm 1", "", "", CmdResult::WrmPath);
    RED_CHECK(cmd_ctx.output_bool);
    RED_CHECK(cmd_ctx.output_message == ""_av);
    check_cmd_ex("wrm off", "", "", CmdResult::WrmPath);
    RED_CHECK(!cmd_ctx.output_bool);
    check_cmd_ex("wrm on", "", "", CmdResult::WrmPath);
    RED_CHECK(cmd_ctx.output_bool);
    RED_CHECK(cmd_ctx.output_message == ""_av);
    check_cmd_ex("wrm on dsadjsdsa.mwrm", "", "", CmdResult::WrmPath);
    RED_CHECK(cmd_ctx.output_bool);
    RED_CHECK(cmd_ctx.output_message == "dsadjsdsa.mwrm"_av);
    check_cmd_ex("wrm newfile.mwrm", "", "", CmdResult::WrmPath);
    RED_CHECK(cmd_ctx.output_bool);
    RED_CHECK(cmd_ctx.output_message == "newfile.mwrm"_av);
    check_cmd_ex("wrm", "", "", CmdResult::WrmPath);
    RED_CHECK(cmd_ctx.output_bool);
    RED_CHECK(cmd_ctx.output_message == ""_av);
    //@}


    // png
    //@{
    check_cmd_ex("png", "", "", CmdResult::Screen);
    RED_CHECK(cmd_ctx.output_message == ""_av);
    check_cmd_ex("png abcde fg.png", "", "", CmdResult::Screen);
    RED_CHECK(cmd_ctx.output_message == "abcde fg.png"_av);
    //@}


    // enable-png
    //@{
    check_cmd_ex("enable-png", "", "", CmdResult::EnableScreen);
    RED_CHECK(cmd_ctx.output_bool);
    check_cmd_ex("enable-png 0", "", "", CmdResult::EnableScreen);
    RED_CHECK(!cmd_ctx.output_bool);
    check_cmd_ex("enable-png 1", "", "", CmdResult::EnableScreen);
    RED_CHECK(cmd_ctx.output_bool);
    check_cmd("enable-png 1 1", "index_param=2 param='1' expected='' type=TooManyArgument", "");
    //@}


    // sid
    //@{
    check_cmd_ex("sid", "", "", CmdResult::Basename);
    RED_CHECK(cmd_ctx.output_message == ""_av);
    check_cmd_ex("sid abcde fg", "", "", CmdResult::Basename);
    RED_CHECK(cmd_ctx.output_message == "abcde fg"_av);
    //@}


    // configfile
    //@{
    check_cmd_ex("f myfile.ini", "", "", CmdResult::ConfigFile);
    RED_CHECK(cmd_ctx.output_message == "myfile.ini"_av);
    check_cmd_ex("f", "index_param=1 param='missing parameter' expected='config filename' type=MissingArgument", "", CmdResult::Fail);
    //@}


    // configstr
    //@{
    check_cmd_ex("conf [section] variable=value", "", "", CmdResult::ConfigStr);
    RED_CHECK(cmd_ctx.output_message == "[section] variable=value"_av);
    check_cmd_ex("conf", "", "", CmdResult::ConfigStr);
    RED_CHECK(cmd_ctx.output_message == ""_av);
    //@}


    // repeat
    //@{
    RED_CHECK(cmd_ctx.repeat_delay == -1);
    RED_CHECK(cmd_ctx.delay.count() == -1);
    check_cmd_ex("repeat 5 my command", "", "", CmdResult::RepetitionCommand);
    RED_CHECK(cmd_ctx.output_message == "my command"_av);
    RED_CHECK(cmd_ctx.repeat_delay == -1);
    RED_CHECK(cmd_ctx.delay.count() == 5000);
    check_cmd_ex("repeat 5000ms 6 my new command", "", "", CmdResult::RepetitionCommand);
    RED_CHECK(cmd_ctx.output_message == "my new command"_av);
    RED_CHECK(cmd_ctx.repeat_delay == 6);
    RED_CHECK(cmd_ctx.delay.count() == 5000);
    check_cmd_ex("repeat 0ms", "", "", CmdResult::RepetitionCommand);
    RED_CHECK(cmd_ctx.output_message == ""_av);
    RED_CHECK(cmd_ctx.repeat_delay == -1);
    RED_CHECK(cmd_ctx.delay.count() == 0);
    check_cmd("repeat -1", "index_param=1 param='-1' expected='delay' type=InvalidFormat", "");
    check_cmd("repeat a", "index_param=1 param='a' expected='delay' type=InvalidFormat", "");
    check_cmd("repeat", "index_param=1 param='missing parameter' expected='delay' type=MissingArgument", "");
    //@}

    // sleep
    //@{
    cmd_ctx.delay = std::chrono::milliseconds(-3);
    check_cmd_ex("sleep", "", "", CmdResult::Ok);
    check_cmd_ex("sleep 0", "", "", CmdResult::Ok);
    check_cmd_ex("sleep 2", "", "", CmdResult::Sleep);
    RED_CHECK(cmd_ctx.delay.count() == 2000);
    check_cmd_ex("sleep 2.4", "", "", CmdResult::Sleep);
    RED_CHECK(cmd_ctx.delay.count() == 2400);
    check_cmd_ex("sleep 2.4ms", "", "", CmdResult::Sleep);
    RED_CHECK(cmd_ctx.delay.count() == 2);
    check_cmd_ex("sleep 5/2", "", "", CmdResult::Sleep);
    RED_CHECK(cmd_ctx.delay.count() == 2500);
    check_cmd_ex("sleep 1/4ms", "", "", CmdResult::Ok);
    RED_CHECK(cmd_ctx.delay.count() == 0);
    check_cmd_ex("sleep 1/4", "", "", CmdResult::Sleep);
    RED_CHECK(cmd_ctx.delay.count() == 250);
    check_cmd_ex("sleep 2s", "", "", CmdResult::Sleep);
    RED_CHECK(cmd_ctx.delay.count() == 2000);
    check_cmd_ex("sleep 2.5s", "", "", CmdResult::Sleep);
    RED_CHECK(cmd_ctx.delay.count() == 2500);
    check_cmd_ex("sleep 1/4s", "", "", CmdResult::Sleep);
    RED_CHECK(cmd_ctx.delay.count() == 250);
    check_cmd_ex("sleep 2min", "", "", CmdResult::Sleep);
    RED_CHECK(cmd_ctx.delay.count() == 2 * 60 * 1000);
    check_cmd_ex("sleep 1min54s6000ms", "", "", CmdResult::Sleep);
    RED_CHECK(cmd_ctx.delay.count() == 2 * 60 * 1000);
    check_cmd_ex("sleep 2m3", "", "", CmdResult::Sleep);
    RED_CHECK(cmd_ctx.delay.count() == 2 * 60 * 1000 + 3000);
    check_cmd_ex("sleep 2m3s", "", "", CmdResult::Sleep);
    RED_CHECK(cmd_ctx.delay.count() == 2 * 60 * 1000 + 3000);
    check_cmd_ex("sleep 2m3ms", "", "", CmdResult::Sleep);
    RED_CHECK(cmd_ctx.delay.count() == 2 * 60 * 1000 + 3);
    check_cmd("sleep -3", "index_param=1 param='-3' expected='delay' type=InvalidFormat", "");
    check_cmd("repeat a", "index_param=1 param='a' expected='delay' type=InvalidFormat", "");
    //@}


    // help
    //@{
    RED_CHECK(CmdResult::OutputResult == cmd_ctx.execute_command("help"_av, mod));
    RED_CHECK(CmdResult::OutputResult == cmd_ctx.execute_command("help sc"_av, mod));
    RED_CHECK(CmdResult::Fail == cmd_ctx.execute_command("help sca"_av, mod));
    //@}


    // quit
    //@{
    check_cmd_ex("q", "", "", CmdResult::Quit);
    check_cmd_ex("quit", "", "", CmdResult::Quit);
    //@}


    // comment
    //@{
    check_cmd("#", "", "");
    check_cmd("#bla bla", "", "");
    check_cmd("# bla bla", "", "");
    check_cmd("  # bla bla", "", "");
    //@}


    #undef check_cmd
}
