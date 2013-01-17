#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>
#include "log.hpp"

void clear_files(const char * path, const char * prefix)
{
    DIR * d = opendir(path);
    if (d){
        size_t path_len = strlen(path);
        size_t prefix_len = strlen(prefix);
        size_t file_len = pathconf(path, _PC_NAME_MAX) + 1;
        char * buffer = (char*)malloc(file_len + path_len + 1);
        strcpy(buffer, path);
        if (buffer[path_len] != '/'){
            buffer[path_len] = '/'; path_len++; buffer[path_len] = 0;
        }
        size_t len = offsetof(struct dirent, d_name) + file_len;
        struct dirent * entryp = (struct dirent *)malloc(len);
        struct dirent * result;
        for (readdir_r(d, entryp, &result) ; result ; readdir_r(d, entryp, &result)) {
            if ((0 == strcmp(entryp->d_name, ".")) || (0 == strcmp(entryp->d_name, ".."))){
                continue;
            }
            if (strncmp(entryp->d_name, prefix, prefix_len)){
                continue;
            }
            strcpy(buffer + path_len, entryp->d_name);
            struct stat st;
            if (stat(buffer, &st) < 0){
                printf("Failed to read file %s [%u: %s]\n", buffer, errno, strerror(errno));
                continue;
            }
            printf("file is %s\n", buffer);
//            if (unlink(buffer) < 0){
//                LOG(LOG_INFO, "Failed to remove file %s", buffer, errno, strerror(errno));
//            }
        }
        closedir(d);
        free(entryp);
        free(buffer);
    }
    else {
        LOG(LOG_INFO, "Failed to open directory %s [%u: %s]", path, errno, strerror(errno));
    }
}


int main(int argc, const char ** argv)
{
    printf("clear_files\n");
    clear_files(argv[1], argv[2]);
}
