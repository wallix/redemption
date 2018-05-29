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
   Copyright (C) Wallix 2010-2013
   Author(s): Clément Moroldo, David Fort
*/

#include "client_redemption/client_input_output_api.hpp"
#include "utils/cli.hpp"


void ClientRedemptionIOAPI::parse_options(int argc, char const* const argv[])
{
    auto options = cli::options(
        cli::helper("Client ReDemPtion Help menu."),

        cli::option('h', "help").help("Show help")
        .action(cli::help),

        cli::option('v', "version").help("Show version")
        .action(cli::quit([]{ std::cout << " ReDemPtion Version " VERSION << "\n"; })),

        cli::helper("========= Connection ========="),

        cli::option('u', "username").help("Set target session user name")
        .action(cli::arg([this](std::string s){
            this->user_name = std::move(s);
            this->commandIsValid += NAME_GOT;
        })),

        cli::option('p', "password").help("Set target session user password")
        .action(cli::arg([this](std::string s){
            this->user_password = std::move(s);
            this->commandIsValid += PWD_GOT;
        })),

        cli::option('i', "ip").help("Set target IP address")
        .action(cli::arg([this](std::string s){
            this->target_IP = std::move(s);
            this->commandIsValid += IP_GOT;
        })),

        cli::option('P', "port").help("Set port to use on target")
        .action(cli::arg([this](int n){
            this->port = n;
            this->commandIsValid += PORT_GOT;
        })),


        cli::helper("========= Verbose ========="),

        cli::option("rdpdr").help("Active rdpdr logs")
        .action(cli::on_off_bit_location<RDPVerbose::rdpdr>(this->verbose)),

        cli::option("rdpsnd").help("Active rdpsnd logs")
        .action(cli::on_off_bit_location<RDPVerbose::rdpsnd>(this->verbose)),

        cli::option("cliprdr").help("Active cliprdr logs")
        .action(cli::on_off_bit_location<RDPVerbose::cliprdr>(this->verbose)),

        cli::option("graphics").help("Active graphics logs")
        .action(cli::on_off_bit_location<RDPVerbose::graphics>(this->verbose)),

        cli::option("printer").help("Active printer logs")
        .action(cli::on_off_bit_location<RDPVerbose::printer>(this->verbose)),

        cli::option("rdpdr-dump").help("Actives rdpdr logs and dump brute rdpdr PDU")
        .action(cli::on_off_bit_location<RDPVerbose::rdpdr_dump>(this->verbose)),

        cli::option("cliprd-dump").help("Actives cliprdr logs and dump brute cliprdr PDU")
        .action(cli::on_off_bit_location<RDPVerbose::cliprdr_dump>(this->verbose)),

        cli::option("basic-trace").help("Active basic-trace logs")
        .action(cli::on_off_bit_location<RDPVerbose::basic_trace>(this->verbose)),

        cli::option("connection").help("Active connection logs")
        .action(cli::on_off_bit_location<RDPVerbose::connection>(this->verbose)),

        cli::option("rail-order").help("Active rail-order logs")
        .action(cli::on_off_bit_location<RDPVerbose::rail_order>(this->verbose)),

        cli::option("asynchronous-task").help("Active asynchronous-task logs")
        .action(cli::on_off_bit_location<RDPVerbose::asynchronous_task>(this->verbose)),

        cli::option("capabilities").help("Active capabilities logs")
        .action(cli::on_off_bit_location<RDPVerbose::capabilities>(this->verbose)),

        cli::option("rail").help("Active rail logs")
        .action(cli::on_off_bit_location<RDPVerbose::rail>(this->verbose)),

        cli::option("rail-dump").help("Actives rail logs and dump brute rail PDU")
        .action(cli::on_off_bit_location<RDPVerbose::rail_dump>(this->verbose)),


        cli::helper("========= Protocol ========="),

        cli::option("vnc").help("Set connection mod to VNC")
        .action([this](){
            this->mod_state = MOD_VNC;
            if (!bool(this->commandIsValid & PORT_GOT)) {
                this->port = 5900;
            }
        }),

        cli::option("rdp").help("Set connection mod to RDP (default).")
        .action([this](){ this->mod_state = MOD_VNC; }),

        cli::option("remote-app").help("Connection as remote application.")
        .action(cli::on_off_bit_location<MOD_RDP_REMOTE_APP>(this->mod_state)),

        cli::option("remote-exe").help("Connection as remote application and set the line command.")
        .action(cli::arg("command", [this](std::string line){
            this->mod_state = MOD_RDP_REMOTE_APP;
            auto pos(line.find(' '));
            if (pos == std::string::npos) {
                this->source_of_ExeOrFile = std::move(line);
                this->source_of_Arguments.clear();
            }
            else {
                this->source_of_ExeOrFile = line.substr(0, pos);
                this->source_of_Arguments = line.substr(pos + 1);
            }
        })),

        cli::option("span").help("Span the screen size on local screen")
        .action(cli::on_off_location(this->is_spanning)),

        cli::option("enable-clipboard").help("Enable clipboard sharing")
        .action(cli::on_off_location(this->enable_shared_clipboard)),

        cli::option("enable-nla").help("Entable NLA protocol")
        .action(cli::on_off_location(this->modRDPParamsData.enable_nla)),

        cli::option("enable-tls").help("Enable TLS protocol")
        .action(cli::on_off_location(this->modRDPParamsData.enable_tls)),

        cli::option("enable-sound").help("Enable sound")
        .action(cli::on_off_location(this->modRDPParamsData.enable_sound)),

        cli::option("enable-fullwindowdrag").help("Enable full window draging")
        .action(cli::on_off_bit_location<~PERF_DISABLE_FULLWINDOWDRAG>(
            this->info.rdp5_performanceflags)),

        cli::option("enable-menuanimations").help("Enable menu animations")
        .action(cli::on_off_bit_location<~PERF_DISABLE_MENUANIMATIONS>(
            this->info.rdp5_performanceflags)),

        cli::option("enable-theming").help("Enable theming")
        .action(cli::on_off_bit_location<~PERF_DISABLE_THEMING>(
            this->info.rdp5_performanceflags)),

        cli::option("enable-cursor-shadow").help("Enable cursor shadow")
        .action(cli::on_off_bit_location<~PERF_DISABLE_CURSOR_SHADOW>(
            this->info.rdp5_performanceflags)),

        cli::option("enable-cursorsettings").help("Enable cursor settings")
        .action(cli::on_off_bit_location<~PERF_DISABLE_CURSORSETTINGS>(
            this->info.rdp5_performanceflags)),

        cli::option("enable-font-smoothing").help("Enable font smoothing")
        .action(cli::on_off_bit_location<PERF_ENABLE_FONT_SMOOTHING>(
            this->info.rdp5_performanceflags)),

        cli::option("enable-desktop-composition").help("Enable desktop composition")
        .action(cli::on_off_bit_location<PERF_ENABLE_DESKTOP_COMPOSITION>(
            this->info.rdp5_performanceflags)),

        cli::option("vnc-applekeyboard").help("Set keyboard compatibility mod with apple VNC server")
        .action(cli::on_off_location(this->vnc_conf.is_apple)),


        cli::helper("========= Client ========="),

        cli::option("width").help("Set screen width")
        .action(cli::arg_location(this->rdp_width)),

        cli::option("height").help("Set screen height")
        .action(cli::arg_location(this->rdp_height)),

        cli::option("bpp").help("Set bit per pixel (8, 15, 16, 24)")
        .action(cli::arg_location("bit_per_pixel", this->info.bpp)),

        cli::option("keylaout").help("Set windows keylayout")
        .action(cli::arg_location(this->info.keylayout)),

        cli::option("enable-record").help("Enable session recording as .wrm movie")
        .action(cli::on_off_location(this->is_recording)),

        cli::option("share-dir").help("Set directory path on local disk to share with your session.")
        .action(cli::arg("directory", [this](std::string s) {
            this->enable_shared_virtual_disk = !s.empty();
            this->SHARE_DIR = std::move(s);
        })),

        cli::option("remote-dir").help("Remote directory")
        .action(cli::arg_location("directory", this->source_of_WorkingDir)),


        cli::helper("========= Replay ========="),

        cli::option('R', "replay").help("Enable replay mode")
        .action(cli::on_off_location(this->is_full_replaying)),

        cli::option("replay-path").help("Set filename path for replay mode")
        .action(cli::arg([this](std::string s){
            this->is_full_replaying = true;
            this->full_capture_file_name = std::move(s);
        }))

    );

    auto cli_result = cli::parse(options, argc, argv);
    switch (cli_result.res) {
        case cli::Res::Ok:
            break;
        case cli::Res::Exit:
            // TODO return 0;
            break;
        case cli::Res::Help:
            cli::print_help(options, std::cout);
            // TODO return 0;
            break;
        case cli::Res::BadFormat:
        case cli::Res::BadOption:
            std::cerr << "Bad " << (cli_result.res == cli::Res::BadFormat ? "format" : "option") << " at parameter " << cli_result.opti;
            if (cli_result.opti < cli_result.argc) {
                std::cerr << " (";
                if (cli_result.res == cli::Res::BadFormat && cli_result.opti > 1) {
                    std::cerr << cli_result.argv[cli_result.opti-1] << " ";
                }
                std::cerr << cli_result.argv[cli_result.opti] << ")";
            }
            std::cerr << "\n";
            // TODO return 1;
            break;
    }
}
