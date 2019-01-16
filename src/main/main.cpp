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
#include "program_options/program_options.hpp"
#include "system/scoped_crypto_init.hpp"
#include "transport/in_file_transport.hpp"
#include "transport/out_file_transport.hpp"

#include "utils/fileutils.hpp"
#include "utils/genfstat.hpp"
#include "utils/genrandom.hpp"
#include "utils/log.hpp"
#include "utils/redemption_info_version.hpp"

#include <iostream>

#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <csignal>

#include <sys/types.h>
#include <dirent.h>


inline void daemonize(const char * pid_file)
{
    int pid;
    int fd;
    char text[256];
    std::size_t lg;

    close(0);
    close(1);
    close(2);

    switch (pid = fork()){
    case -1:
        std::clog << "problem forking "
        << errno << ":'" << strerror(errno) << "'\n";
        _exit(1);
    default: /* exit, this is the main process (daemonizer) */
        _exit(0);
    case 0: /* child daemon process */
        pid = getpid();
        fd = open(pid_file, O_WRONLY | O_CREAT, S_IRWXU);
        if (fd == -1) {
            std::clog
            <<  "Writing process id to " LOCKFILE " failed. Maybe no rights ?"
            << " : " << errno << ":'" << strerror(errno) << "'\n";
            _exit(1);
        }
        lg = snprintf(text, 255, "%d", pid);

        try {
            OutFileTransport(unique_fd{fd}).send(text, lg);
        } catch (Error const &) {
            LOG(LOG_ERR, "Couldn't write pid to %s: %s", pid_file, strerror(errno));
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


inline int shutdown()
{
    const char * const pid_file = app_path(AppPath::LockFile);
    const char * const pid_dir = app_path(AppPath::LockDir);

    std::cout <<
      "Stopping rdpproxy\n"
      "Looking if pid_file " << pid_file << " exists\n"
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
        std::cerr << "Failed to read pid file. " << strerror(errno) << ".\n";
        return 1; // file does not exist.
    }

    try {
        std::cout << "Reading pid_file " << pid_file << "\n";
        char text[256];

        text[InFileTransport(std::move(fd)).partial_read(text, sizeof(text)-1)] = 0;

        int pid = atoi(text); /*NOLINT*/
        std::cout << "Stopping process id " << pid << "\n";

        // check name of pid
        if (pid > 0)
        {
            char path[64];
            std::sprintf(path, "/proc/%d/cmdline", pid);
            unique_fd fd(open(path, O_RDONLY));
            if (!fd.is_open()) {
                std::cerr << path << ": " << strerror(errno) << "\n";
                pid = 0;
            }
            else {
                bool is_proxy = true;
                try {
                    text[InFileTransport(std::move(fd)).partial_read(text, sizeof(text)-1)] = 0;
                    if (!strstr(text, "/rdpproxy")) {
                        is_proxy = false;
                    }
                }
                catch (...) {
                    is_proxy = false;
                }

                if (!is_proxy) {
                    std::cerr << "Error process id " << pid << " is not a rdpproxy\n";
                    pid = 0;
                }
            }
        }

        if (pid > 0) {
            int res = kill(pid, SIGTERM);
            if (res != 0) {
                if (errno != ESRCH) {
                    int err = errno;
                    std::cerr << "Error stopping process id " << pid << ": " << strerror(err) << "\n";
                }
                else {
                    // errno != ESRCH, pid is still running
                    std::cout << "Process " << pid << " is still running, let's send a KILL signal\n";
                    res = kill(pid, SIGKILL);
                    if (res != 0){
                        std::cerr << "Error stopping process id " << pid << "\n";
                    }
                }
            }
        }
    }
    catch (Error const&) {
        std::cerr << "Failed to read pid file. " << strerror(errno) << "\n";
    }
    unlink(pid_file);

    // remove all other pid files
    DIR * d = opendir(pid_dir);
    if (d){
        const std::string path = pid_dir;
        for (dirent * entryp = readdir(d) ; entryp ; entryp = readdir(d)) {
            if ((0 == strcmp(entryp->d_name, ".")) || (0 == strcmp(entryp->d_name, ".."))){
                continue;
            }
            const std::string pidpath = path + "/" + entryp->d_name;
            LOG(LOG_INFO, "removing old pid file %s", pidpath.c_str());
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
    const char * const copyright_notice = "\n"
        "A Remote Desktop Protocol proxy.\n"
        "Copyright (C) WALLIX 2010-2018.\n"
        "Christophe Grosjean, Javier Caverni, Xavier Dunat, Olivier Hervieu,\n"
        "Martin Potier, Dominique Lafages, Jonathan Poelen, Raphael Zhou,\n"
        "Meng Tan, Clement Moroldo and Jennifer Inthavongh.";

    setlocale(LC_CTYPE, "C");

    const unsigned uid = getuid();
    const unsigned gid = getgid();

    unsigned euid = uid;
    unsigned egid = gid;

    std::string config_filename = app_path(AppPath::CfgIni);

    static constexpr char const * opt_print_ini_spec = "print-spec";
    static constexpr char const * opt_print_ini = "print-default-ini";

    program_options::options_description desc({
        {'h', "help", "produce help message"},
        {'v', "version", "show software version"},
        {'k', "kill", "shut down rdpproxy"},
        {'n', "nodaemon", "don't fork into background"},

        {'u', "uid", &euid, "run with given uid"},

        {'g', "gid", &egid, "run with given gid"},

        //{'t', "trace", "trace behaviour"},

        {'c', "check", "check installation files"},

        {'f', "force", "remove application lock file"},

        {'i', "inetd", "launch redemption with inetd like launcher"},

        {'N', "nofork", "not forkable (debug)"},

        {"config-file", &config_filename, "use an another ini file"},

        {opt_print_ini_spec, "Show file spec for rdpproxy.ini"},
        {opt_print_ini, "Show default rdpproxy.ini"}

        //{"test", "check Inifile syntax"}
    });

    program_options::variables_map options;

    try {
        options = program_options::parse_command_line(argc, argv, desc);
    }
    catch (std::exception const& e) {
        std::cerr << argv[0] << ": error: " << e.what() << "\n";
        return 17;
    }

    if (options.count("kill")) {
        int status = shutdown();
        if (status){
            // TODO check the real error that occured
            std::clog << "problem opening " << app_path(AppPath::LockFile) << "."
            " Maybe rdpproxy is not running\n";
        }
        return status;
    }

    if (options.count("help")) {
        std::cout << redemption_info_version() << copyright_notice << "\n\n";
        std::cout << "Usage: rdpproxy [options]\n\n";
        std::cout << desc << std::endl;
        return 0;
    }

    if (options.count("version")) {
        std::cout << redemption_info_version() << copyright_notice << std::endl;
        return 0;
    }

    if (options.count(opt_print_ini_spec)) {
        std::cout <<
            #include "configs/autogen/str_python_spec.hpp"
        ;
        return 0;
    }
    if (options.count(opt_print_ini)) {
        std::cout <<
            #include "configs/autogen/str_ini.hpp"
        ;
        return 0;
    }

    openlog("rdpproxy", LOG_CONS | LOG_PERROR, LOG_USER);

    if (options.count("check")) {
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


    CryptoContext cctx;
    UdevRandom rnd;
    Fstat fstat;

    if (options.count("inetd")) {
        redemption_new_session(cctx, rnd, fstat, config_filename.c_str());
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

    if (options.count("force")){
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


    /* write the pid to file */
    int const fd = open(app_path(AppPath::LockFile), O_WRONLY | O_CREAT, S_IRWXU);
    if (fd == -1) {
        std::clog
        <<  "Writing process id to " << app_path(AppPath::LockFile)
        << " failed. Maybe no rights ?" << " : " << errno << ":'" << strerror(errno) << "'\n";
        return 1;
    }

    try {
        char text[256];
        size_t lg = snprintf(text, 255, "%d", getpid());
        OutFileTransport(unique_fd{fd}).send(text, lg);
    } catch (Error const &) {
        LOG(LOG_ERR, "Couldn't write pid to %s: %s", app_path(AppPath::LockFile), strerror(errno));
        return 1;
    }

    if (!options.count("nodaemon")) {
        daemonize(app_path(AppPath::LockFile));
    }

    Inifile ini;
    configuration_load(ini.configuration_holder(), config_filename);

    ScopedCryptoInit scoped_crypto;

    if (!ini.get<cfg::globals::enable_transparent_mode>()) {
        if (setgid(egid) != 0){
            LOG(LOG_ERR, "Changing process group to %u failed with error: %s\n", egid, strerror(errno));
            return 1;
        }
        if (setuid(euid) != 0){
            LOG(LOG_ERR, "Changing process user to %u failed with error: %s\n", euid, strerror(errno));
            return 1;
        }
    }

    if (bool(ini.get<cfg::video::capture_flags>() & CaptureFlags::ocr)
     && ini.get<cfg::ocr::version>() == OcrVersion::v2
    ) {
        // load global constant...
        rdp_ppocr::get_ocr_constants(
            app_path(AppPath::Cfg),
            static_cast<ocr::locale::LocaleId::type_id>(ini.get<cfg::ocr::locale>())
        );
    }

    LOG(LOG_INFO, "ReDemPtion " VERSION " starting");
    redemption_main_loop(
        ini, cctx, rnd, fstat, euid, egid,
        std::move(config_filename),
        !options.count("nofork"));

    /* delete the .pid file if it exists */
    /* don't care about errors. */
    /* If we are not in daemon mode this file will not exists, */
    /* hence some errors are expected */
    unlink(app_path(AppPath::LockFile));

    return 0;
}
