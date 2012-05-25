/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Xavier Dunat,
              Olivier Hervieu, Martin Potier
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   main program

*/

#include <unistd.h>
#include <locale.h>
#include <stdio.h>
#include <signal.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <iostream>
#include "version.hpp"

#include "constants.hpp"

#include <fstream>

#include "check_files.hpp"
#include "mainloop.hpp"
#include "log.hpp"


void daemonize(const char * pid_file)
{
    int pid;
    int fd;
    char text[256];
    size_t lg;

    using namespace std;

    close(0);
    close(1);
    close(2);

    switch (pid = fork()){
    case -1:
        clog << "problem forking "
        << errno << ":'" << strerror(errno) << "'\n";
        _exit(1);
    default: /* exit, this is the main process (daemonizer) */
        _exit(0);
    case 0: /* child daemon process */
        pid = getpid();
        fd = open(pid_file, O_WRONLY | O_CREAT, S_IRWXU);
        if (fd == -1) {
            clog
            <<  "Writing process id to " LOCKFILE " failed. Maybe no rights ?"
            << " : " << errno << ":'" << strerror(errno) << "'\n";
            _exit(1);
        }
        lg = snprintf(text, 255, "%d", pid);
        if (write(fd, text, lg) == -1) {
            LOG(LOG_ERR, "Couldn't write pid to %s: %s", PID_PATH "/" LOCKFILE, strerror(errno));
            _exit(1);
        }
        close(fd);
        usleep(1000000);
        open("/dev/null", O_RDONLY, 0); // stdin  0
        open("/dev/null", O_WRONLY, 0); // stdout 1
        open("/dev/null", O_WRONLY, 0); // stderr 2

        clog << "Process " << pid << " started as daemon\n";
    }
}

/*****************************************************************************/
int shutdown(const char * pid_file)
{
    int pid;
    int fd;
    char text[256];

    using namespace std;

    text[0] = 0;
    cout << "stopping rdpproxy\n";
    /* read the rdpproxy.pid file */
    fd = -1;
    cout << "looking if pid_file " << pid_file <<  " exists\n";
    if ((0 == access(pid_file, F_OK))) { /* rdpproxy.pid */
        /*Code related to g_file_open os_call*/
        fd =  open(pid_file, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if (fd == -1) {
            /* can't open read / write, try to open read only */
            fd =  open(pid_file, O_RDONLY);
        }
    }
    if (fd == -1) {
        return 1;
    }
    cout << "reading pid_file " << pid_file << "\n";
    memset(text, 0, 32);
    if (read(fd, text, 31) < 0){
        cout << "failed to read pid file\n";
        cout << "pid =" << text << "\n";
    }
    else{
        pid = atoi(text);
        cout << "stopping process id " << pid << "\n";
        if (pid > 0) {
            TODO("check that a process with this id is really running, if not we may consider removing pid file.")
            cout << "sending sigterm to " << pid << "\n";
            kill(pid, SIGKILL);
            TODO("wait some decent time and check no such process is running any more")
        }
    }
    TODO("exit with a correct error status if something failed")
    close(fd);
    unlink(pid_file);
    return 0;
}

#include <boost/program_options.hpp>

namespace po = boost::program_options;

const char * copyright_notice =
"\n"
"Redemption " VERSION ": A Remote Desktop Protocol proxy.\n"
"Copyright (C) Wallix 2010-2012.\n"
"Christophe Grosjean, Javier Caverni, Xavier Dunat, Olivier Hervieu,\n"
"Martin Potier, Dominique Lafages and Jonathan Poelen\n"
"\n"
;

int main(int argc, char** argv)
{
    bool check_share_files = false;
    bool check_etc_files = false;
    int fd;
    int pid;
    char text[256];
    setlocale(LC_CTYPE, "C");
    Check_files cfc;

    unsigned uid = getuid();
    unsigned gid = getgid();

    po::options_description desc("Options");
    desc.add_options()
    // -help, --help, -h
    ("help,h", "produce help message")
    // -help, --help, -h
    ("version,v", "show software version")
    // -kill, --kill, -k
    ("kill,k", "shut down rdpproxy")
    // -nodaemon" -n
    ("nodaemon,n", "don't fork into background")

    ("uid,u", po::value<unsigned>(&uid), "run with given uid")

    ("gid,g", po::value<unsigned>(&gid), "run with given gid")

//    ("trace,t", "trace behaviour")

    ("check,c", "check installation files")

    ("force,f", "remove application lock file")

    // -inetd, --inetd, -i
    ("inetd,i", "launch redemption with inetd like launcher")

//    ("test", "check Inifile syntax")
    ;

    using namespace std;

    po::variables_map options;
    po::store(po::parse_command_line(argc, argv, desc), options);
    po::notify(options);

    if (options.count("kill")) {
        int status = shutdown(PID_PATH "/" LOCKFILE);
        if (status){
            TODO("check the real error that occured")
            clog << "problem opening " << PID_PATH "/" LOCKFILE  << "."
            " Maybe rdpproxy is not running\n";
        }
        _exit(status);
    }
    if (options.count("help")) {
        cout << copyright_notice;
        cout << "Usage: rdpproxy [options]\n\n";
        cout << desc << endl;
        _exit(0);
    }
    if (options.count("version")) {
        cout << copyright_notice;
        cout << "Version " VERSION "\n" << endl;
        _exit(0);
    }
    if (options.count("check")) {
        check_share_files = cfc.check_share();
        check_etc_files = cfc.check_etc();
        std::clog << boolalpha;
        clog <<
        LOGIN_LOGO24   " is present at " SHARE_PATH     " .... " << cfc.ad24b       << "\n"
        CURSOR0        " is present at " SHARE_PATH     " .... " << cfc.cursor0     << "\n"
        CURSOR1        " is present at " SHARE_PATH     " .... " << cfc.cursor1     << "\n"
        FONT1          " is present at " SHARE_PATH     " .... " << cfc.sans        << "\n"
        REDEMPTION_LOGO24  " is present at " SHARE_PATH " .... " << cfc.logo        << "\n"
        RSAKEYS_INI    " is present at " CFG_PATH       "..... " << cfc.keys        << "\n"
        RDPPROXY_INI   " is present at " CFG_PATH       "..... " << cfc.config_file << "\n"
        ;

        if (!(check_share_files && check_etc_files)){
            std::clog << boolalpha;
            clog <<
            "Share files test result: " << check_share_files << ".\n"
            << "Etc files test result: " << check_etc_files <<  ".\n"
            << "Please verify that all tests passed. If not, "
            "you may need to remove " PID_PATH "/" LOCKFILE " or "
            "reinstall rdpproxy if some configuration files are missing.\n";
        }
        _exit(0);
    }

    openlog("rdpproxy", LOG_CONS | LOG_PERROR, LOG_USER);

    if (options.count("inetd")) {
        redemption_new_session();
        exit(0);
    }

    // if -f (force option) is set
    // force kill running rdpproxy
    // force remove pid file
    // don't check if it fails (proxy may be allready stopped)
    // and try to continue normal start process afterward

    if (options.count("force")){
        shutdown(PID_PATH  "/" LOCKFILE);
    }

    if (0 == access(PID_PATH "/" LOCKFILE, F_OK)) {
        clog <<
        "File " << PID_PATH "/" LOCKFILE << " already exists. "
        "It looks like rdpproxy is already running, "
        "if not delete the " LOCKFILE " file and try again\n";
        _exit(0);
    }
    /* write the pid to file */
    fd = open(PID_PATH "/" LOCKFILE, O_WRONLY | O_CREAT, S_IRWXU);
    if (fd == -1) {
        clog
        <<  "Writing process id to " LOCKFILE " failed. Maybe no rights ?"
        << " : " << errno << ":'" << strerror(errno) << "'\n";
        _exit(1);
    }
    pid = getpid();
    size_t lg = snprintf(text, 255, "%d", pid);
    if (write(fd, text, lg) == -1) {
        LOG(LOG_ERR, "Couldn't write pid to %s: %s", PID_PATH "/" LOCKFILE, strerror(errno));
        _exit(1);
    }
    close(fd);

    if (!options.count("nodaemon")) {
        daemonize(PID_PATH "/" LOCKFILE);
    }

    setuid(uid);
    setgid(gid);

    LOG(LOG_INFO, "ReDemPtion " VERSION " starting");
    redemption_main_loop();

    /* delete the .pid file if it exists */
    /* don't care about errors. */
    /* If we are not in daemon mode this file will not exists, */
    /* hence some errors are expected */
    unlink(PID_PATH "/" LOCKFILE);

    return 0;
}
