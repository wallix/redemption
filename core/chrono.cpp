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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <cstdlib>

#include "rdtsc.hpp"


struct chrono{
    char file[256];
    long long unsigned cycles_start;
    long long unsigned cycles_end;
    long long unsigned cycles_result;
    time_t seconds_start;
    time_t seconds_end;
    double seconds_result;
    float cycles_seconds;

    chrono(const char profile_file[256]){
        this->cycles_start = 0;
        this->cycles_end = 0;
        this->cycles_result = 0;
        this->seconds_start = 0;
        this->seconds_end = 0;
        this->seconds_result = 0.0;
        this->cycles_seconds = 0.0;
        strncpy(this->file, profile_file, 256);
    }

    chrono(){
        this->cycles_start = 0;
        this->cycles_end = 0;
        this->cycles_result = 0;
        this->seconds_start = 0;
        this->seconds_end = 0;
        this->seconds_result = 0.0;
        this->cycles_seconds = 0.0;
        strncpy(this->file, "/var/log/rdpproxy_profile.log", 256);
    }

    ~chrono(){}

    void begin(){
        this->cycles_start = rdtsc();
         time((&this->seconds_start));
    }

    void finish(){
        this->cycles_end = rdtsc();
        time(&(this->seconds_end));
    }

    void cycles_count(const char function_name[256]){
        FILE * fd;
        char text[256];

        time_t rawtime;
        struct tm * timeinfo;

        time ( &rawtime );
        timeinfo = localtime ( &rawtime );

        this->cycles_result = this->cycles_end - this->cycles_start;


        fd = fopen(this->file, "a");
        if (fd == NULL){
            printf("Error opening the file");
            return;
        }
        fprintf (fd, "The current date/time is: %s", asctime (timeinfo) );
        fprintf(fd, "\t%s", function_name);
        snprintf(text, 255, " : %llu cycles on this function\n", this->cycles_result);
        fprintf(fd, "%s", text);
        fclose(fd);
        return;
    }

    void cycles_to_seconds(char function_name[256]){

        FILE * fd;
        FILE * fd_var;
        long lSize;
        char * pch;
        char * buffer;
        char text[256];
        float freq;

        /* Obtaining CPU Processor info*/
        system("cat /proc/cpuinfo > cpuinfo.log");

        fd = fopen("cpuinfo.log", "r");
        if (fd == NULL){
            printf("Error opening the file");
            return;
        }
        fseek (fd , 0 , SEEK_END);
        lSize = ftell (fd);
        rewind (fd);

        buffer = (char*) malloc (sizeof(char)*lSize);
        fread (buffer,1,lSize,fd);

        pch = strtok (buffer,"@");
        pch = strtok (NULL, " G");
        /* freq is the CPU frequency of the machine */
        freq = atof(pch);

        fclose(fd);

        fd_var = fopen(this->file, "a");
        if (fd_var == NULL){
            printf("Error opening the file");
            return;
        }

        /* Expressing frequency in Hz */

        freq = freq * 1E9;
        if (freq != 0){
            this->cycles_seconds = this->cycles_result / freq;
        }
        else{
            fprintf(fd_var, "Error recovering CPU frequency info\n");
            unlink("cpuinfo.log");
            return;
        }

        fprintf(fd_var, "\t%s", function_name);
        snprintf(text, 255, " : %.10f seconds spent in this function based on number of cycles\n", this->cycles_seconds);
        fprintf(fd_var, "%s", text);
        fclose(fd_var);
        unlink("cpuinfo.log");
    }

    void seconds_count(char function_name[256]){

        FILE * fd;
        char text[256];

        this->seconds_result = difftime(this->seconds_end, this->seconds_start);

        fd = fopen(this->file, "a");
        if (fd == NULL){
            printf("Error opening the file");
            return;
        }
        fprintf(fd, "\t%s", function_name);
        snprintf(text, 255, " : %.10lf seconds spent in this function\n", this->seconds_result);
        fprintf(fd, "%s", text);
        fclose(fd);
    }
};
