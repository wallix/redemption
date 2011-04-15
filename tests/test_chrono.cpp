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
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

*/

#include "chrono.cpp"


void write_time_in_file(){

    FILE * fd;

    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    fd = fopen("test.log", "a");
    if (fd == NULL){
        printf("Error opening the file");
        return;
    }
    fprintf (fd, "The current date/time is: %s", asctime (timeinfo) );
    fclose(fd);
    unlink("test.log");
}


int main(int argc, char** argv)
{
    /* In chrono constructor has no parameters, profile file specified
    by default is "/var/log/rdpproxy_profile.log". If you're not sure
    chrono's object is being used as sudoer, you must specify another
    file in order to save your profiling results as it's shown in that
    user case */
    chrono profil("/tmp/test_profile.log");

    /* First thing to do in order to profile one function, initialise
    cycle counter and time counter */
    profil.begin();

    /* Execute function to be profiled */

    write_time_in_file();

    /* Fixed end of cycle counter and time counter and printing results at
    file specified at chrono's constuctor */

    profil.finish();
    profil.cycles_count("write_time_in_file");

    return 0;
}
