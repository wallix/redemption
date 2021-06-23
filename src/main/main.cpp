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
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
              Olivier Hervieu, Martin Potier, Raphael Zhou
              and Meng Tan

   main program
*/

#include "capture/cryptofile.hpp"
#include "capture/ocr/locale/locale_id.hpp"
#include "capture/rdp_ppocr/get_ocr_constants.hpp"

#include "configs/config.hpp"

#include "core/check_files.hpp"
#include "core/mainloop.hpp"

#include "main/version.hpp"
#include "system/scoped_crypto_init.hpp"
#include "system/scoped_ssl_init.hpp"
#include "transport/file_transport.hpp"

#include "utils/fileutils.hpp"
#include "utils/log.hpp"
#include "utils/redemption_info_version.hpp"
#include "utils/strutils.hpp"
#include "utils/sugar/int_to_chars.hpp"
#include "utils/cli.hpp"

#include <iostream>
#include <charconv>

#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <csignal>

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h> // sleep


static bool write_pid_file(int pid)
{
    char const* pid_file = app_path(AppPath::LockFile);
    int fd = open(pid_file, O_WRONLY | O_CREAT, S_IRWXU);
    if (fd == -1) {
        int errnum = errno;
        std::clog <<  "Writing process id to " << pid_file << " failed. Maybe no rights ?"
                  << " : " << errnum << ":'" << strerror(errnum) << "'\n";
        return false;
    }

    auto text = int_to_decimal_chars(pid);

    if (write(fd, text.data(), text.size()) != ssize_t(text.size())) {
        LOG(LOG_ERR, "Couldn't write pid to %s: %s", pid_file, strerror(errno));
        return false;
    }

    return true;
}

static void daemonize()
{
    close(0);
    close(1);
    close(2);

    switch (fork()){
    case -1:
        std::clog << "problem forking "
        << errno << ":'" << strerror(errno) << "'\n";
        _exit(1);
    default: /* exit, this is the main process (daemonizer) */
        _exit(0);
    case 0: /* child daemon process */
        int pid = getpid();

        if (!write_pid_file(pid)) {
            _exit(1);
        }

        {
            timespec req={0, 1000000000L};
            nanosleep(&req, nullptr/*req*/);
        }

        open("/dev/null", O_RDONLY, 0); // stdin  0
        open("/dev/null", O_WRONLY, 0); // stdout 1
        open("/dev/null", O_WRONLY, 0); // stderr 2

        std::clog << "Process " << pid << " started as daemon\n";
    }
}


static int shutdown()
{
    auto const pid_file = app_path(AppPath::LockFile);

    std::cout <<
      "Stopping rdpproxy\n"
      "Looking if pid_file " << pid_file.to_sv() << " exists\n"
    ;

    /* read the rdpproxy.pid file */
    unique_fd fd = invalid_fd();
    if ((0 == access(pid_file, F_OK))) {
        fd = unique_fd(open(pid_file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR));
        if (!fd) {
            /* can't open read / write, try to open read only */
            fd = unique_fd(open(pid_file, O_RDONLY));
        }
    }

    if (!fd) {
        int err = errno;
        std::cerr << "Failed to read pid file. " << strerror(err) << ".\n";
        return 1; // file does not exist.
    }

    // kill rdpproxy process
    [&] {
        char buffer[256];
        ssize_t n = read(fd.fd(), buffer, sizeof(buffer));

        if (n < 0) {
            int err = errno;
            std::cerr << "Failed to read pid file. " << strerror(err) << "\n";
            return ;
        }

        // check name of pid
        int pid;
        const auto r = std::from_chars(buffer, buffer+n, pid);
        if (r.ec != std::errc() || pid < 0) {
            return ;
        }

        // auto path = "/proc/%d/cmdline"_static_fmt(pid);
        char path[256];
        std::sprintf(path, "/proc/%d/cmdline", pid);
        fd = unique_fd(open(path, O_RDONLY));
        if (!fd.is_open()) {
            int err = errno;
            std::cerr << path << ": " << strerror(err) << "\n";
            return ;
        }

        n = read(fd.fd(), buffer, sizeof(buffer)-1);
        if (n <= 0) {
            return ;
        }

        buffer[n] = 0;
        if (!strstr(buffer, "/rdpproxy")) {
            std::cerr << "Error process id " << pid << " is not a rdpproxy\n";
            return ;
        }

        int res = kill(pid, SIGTERM);
        if (res != 0) {
            if (errno == ESRCH) {
                // pid is still running
                std::cout << "Process " << pid << " is still running, let's send a KILL signal\n";
                res = kill(pid, SIGKILL);
            }

            if (res != 0) {
                int err = errno;
                std::cerr << "Error stopping process id " << pid << ": " << strerror(err) << "\n";
                return ;
            }
        }

        // process is killed, wait a bit for the sockets to close
        sleep(1);
    }();

    unlink(pid_file);

    // remove all other pid files
    auto const pid_dir = app_path(AppPath::LockDir);
    DIR * d = opendir(pid_dir);
    if (d){
        std::string pidpath;
        for (dirent * entryp = readdir(d) ; entryp ; entryp = readdir(d)) {
            if (dirname_is_dot(entryp->d_name)) {
                continue;
            }
            str_assign(pidpath, pid_dir, '/', entryp->d_name);
            LOG(LOG_INFO, "removing old pid file %s", pidpath);
            if (unlink(pidpath.c_str()) < 0){
                LOG(LOG_ERR, "Failed to remove old session pid file %s [%d: %s]",
                    pidpath.c_str(), errno, strerror(errno));
            }
        }
        closedir(d);
    }
    else {
        LOG(LOG_ERR, "Failed to open dynamic configuration directory %s [%d: %s]",
            pid_dir, errno, strerror(errno));
    }

    return 0;
}


int main(int argc, char** argv)
{
    setlocale(LC_CTYPE, "C");

    const unsigned uid = getuid();
    const unsigned gid = getgid();

    unsigned euid = uid;
    unsigned egid = gid;

    bool is_nodeamon = false;

    std::string config_filename = app_path(AppPath::CfgIni).to_string();

    bool enable_check = false;
    bool enable_kill = false;
    bool enable_force = false;
    bool enable_nofork = false;

    auto const options = cli::options(
        cli::option('h', "help").help("produce help message")
            .parser(cli::help()),

        cli::option('v', "version").help("show software version")
            .parser(cli::quit([]{
                std::cout
                    << redemption_info_version() << "\n"
                    << redemption_info_copyright() << std::endl
                ;
            })),

        cli::option('k', "kill").help("shut down rdpproxy")
            .parser(cli::on_off_location(enable_kill)),

        cli::option('n', "nodaemon").help("don't fork into background")
            .parser(cli::on_off_location(is_nodeamon)),

        cli::option('u', "uid").help("run with given uid")
            .parser(cli::arg_location(euid)).argname("<uid>"),

        cli::option('g', "gid").help("run with given gid")
            .parser(cli::arg_location(egid)).argname("<gid>"),

        cli::option('c', "check").help("check installation files")
            .parser(cli::on_off_location(enable_check)),

        cli::option('f', "force").help("remove application lock file")
            .parser(cli::on_off_location(enable_force)),

        cli::option('N', "nofork").help("not forkable (debug)")
            .parser(cli::on_off_location(enable_nofork)),

        cli::option("config-file").help("use an another ini file")
            .parser(cli::arg_location(config_filename)).argname("<path>"),

        cli::option("print-spec").help("Show file spec for rdpproxy.ini")
            .parser(cli::quit([]{
                std::cout <<
                    #include "configs/autogen/str_python_spec.hpp"
                ;
            })),

        cli::option("print-rdp-cp-spec").help("Show connection policy spec for rdp protocol")
            .parser(cli::quit([]{
                std::cout <<
                    #include "configs/autogen/rdp_cp_spec.hpp"
                ;
            })),

        cli::option("print-vnc-cp-spec").help("Show connection policy spec for vnc protocol")
            .parser(cli::quit([]{
                std::cout <<
                    #include "configs/autogen/vnc_cp_spec.hpp"
                ;
            })),

        cli::option("print-cp-mapping").help("Show connection policy mapping for sesman")
            .parser(cli::quit([]{
                std::cout <<
                    #include "configs/autogen/cp_mapping.hpp"
                ;
            })),

        cli::option("print-default-ini").help("Show default rdpproxy.ini")
            .parser(cli::quit([]{
                std::cout <<
                    #include "configs/autogen/str_ini.hpp"
                ;
            }))
    );

    auto const cli_result = cli::parse(options, argc, argv);
    switch (cli_result.res) {
        case cli::Res::Ok:
            break;
        case cli::Res::Exit:
            return 0;
        case cli::Res::Help:
            std::cout << "Usage: rdpproxy [options]\n\n";
            cli::print_help(options, std::cout);
            return 0;
        case cli::Res::BadFormat:
        case cli::Res::BadOption:
        case cli::Res::NotOption:
        case cli::Res::StopParsing:
            std::cerr << "Bad " << (cli_result.res == cli::Res::BadFormat ? "format" : "option") << " at parameter " << cli_result.opti;
            if (cli_result.opti < cli_result.argc) {
                std::cerr << " (" << cli_result.argv[cli_result.opti] << ")";
            }
            std::cerr << "\n";
            return 17;
    }

    openlog("rdpproxy", LOG_CONS | LOG_PERROR, LOG_USER);

    if (enable_kill) {
        int status = shutdown();
        if (status){
            // TODO check the real error that occured
            std::clog << "problem opening " << app_path(AppPath::LockFile) << "."
            " Maybe rdpproxy is not running\n";
        }
        return status;
    }

    if (enable_check) {
        /*
          setgid(egid);
          setuid(euid);
        */
        bool const euser_check_file_result = check_files(euid, egid);
        /*
          setgid(gid);
          setuid(uid);
        */

        if (!euser_check_file_result)
        {
            LOG(LOG_INFO,
                "Please verify that all tests passed. If not, you may need "
                "to remove %s or reinstall rdpproxy if some configuration "
                "files are missing.", app_path(AppPath::LockFile));
            return 1;
        }

        return 0;
    }


    // if -f (force option) is set
    // force kill running rdpproxy
    // force remove pid file
    // don't check if it fails (proxy may be allready stopped)
    // and try to continue normal start process afterward

    if (!file_exist(app_path(AppPath::LockDir)) && recursive_create_directory(app_path(AppPath::LockDir), 0700, egid) < 0){
        LOG(LOG_ERR, "Failed to create %s: %s", app_path(AppPath::LockDir), strerror(errno));
        return 1;
    }

    if (chown(app_path(AppPath::LockDir), euid, egid) < 0){
        LOG(LOG_ERR, "Failed to set owner %u.%u to %s", euid, egid, app_path(AppPath::LockDir));
        return 1;
    }

    if (enable_force){
        shutdown();
    }

    if (0 == access(app_path(AppPath::LockFile), F_OK)) {
        std::clog <<
        "File " << app_path(AppPath::LockFile) << " already exists. "
        "It looks like rdpproxy is already running, "
        "if not, try again with -f (force) option or delete the "
        << app_path(AppPath::LockFile) << " file and try again\n";
        return 1;
    }

    if (!write_pid_file(getpid())) {
        return 1;
    }

    if (!is_nodeamon) {
        daemonize();
    }

    Inifile ini;
    configuration_load(ini.configuration_holder(), config_filename.c_str());

    ScopedCryptoInit scoped_crypto;
    ScopedSslInit scoped_ssl;

    if (!ini.get<cfg::globals::enable_transparent_mode>()) {
        if (setgid(egid) != 0){
            LOG(LOG_ERR, "Changing process group to %u failed with error: %s", egid, strerror(errno));
            return 1;
        }
        if (setuid(euid) != 0){
            LOG(LOG_ERR, "Changing process user to %u failed with error: %s", euid, strerror(errno));
            return 1;
        }
    }

    if (bool(ini.get<cfg::video::capture_flags>() & CaptureFlags::ocr)
     && ini.get<cfg::ocr::version>() == OcrVersion::v2
    ) {
        // load global constant...
        rdp_ppocr::get_ocr_constants(
            app_path(AppPath::Cfg).to_string(),
            static_cast<ocr::locale::LocaleId::type_id>(ini.get<cfg::ocr::locale>())
        );
    }

    LOG(LOG_INFO, "ReDemPtion " VERSION " starting");
    redemption_main_loop(
        ini, euid, egid,
        std::move(config_filename),
        !enable_nofork);

    /* delete the .pid file if it exists */
    /* don't care about errors. */
    /* If we are not in daemon mode this file will not exists, */
    /* hence some errors are expected */
    unlink(app_path(AppPath::LockFile));

    return 0;
}
