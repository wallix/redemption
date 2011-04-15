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
   Author(s): Christophe Grosjean, Xavier Dunat

   Module: authhook.cpp
   Description : sample post-login authentication data processing

   This code is not actually used in production code, it's just a
   minimal tcp server sample whose purpose is to test the protocol.
   It shows how the post-processing of authentication data works.

   If no such hook is provided redemption is supposed to works seamlessly
   on it's own with consistant behavior.

*/

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "stream.hpp"
#include "config.hpp"

int keepalived = 0;
int warningMessageValidated = 0;
int recordingMessageValidated = 2;
int authenticated = 0;

void checkKey(const char * key, const char * value)
{
    if (strcmp(key, STRAUTHID_KEEPALIVE)==0) {
        printf("keepalived\n");
        keepalived=1;
    }
    else if (strcmp(key, STRAUTHID_ACCEPT_MESSAGE)==0){
        printf("recording message validated\n");
        if (strcmp(value, "True")==0){
            recordingMessageValidated = 1;
        }
        else {
            recordingMessageValidated = 2;
        }
    }
    else if (strcmp(key, STRAUTHID_DISPLAY_MESSAGE)==0) {
        printf("warning message validated\n");
        warningMessageValidated = 1;
    }
    else if ((strcmp(key, STRAUTHID_PASSWORD)==0) && strcmp(value, "ASK")){
        printf("warning message validated\n");
        authenticated = 1;
    }
}

void addKeyValue(Stream *stream, const char * key, const char * value)
{
    stream->out_copy_bytes(key, strlen(key));
    stream->out_uint8('\n');
    stream->out_copy_bytes(value, strlen(value));
    stream->out_uint8('\n');
    printf("envoi : %s:%s\n", key, value);
}

void askAuthentication(Stream *stream)
{
    addKeyValue(stream, STRAUTHID_PASSWORD, "ASK");
    addKeyValue(stream, STRAUTHID_TRANS_HELP_MESSAGE, "Ceci est le message d'aide<br><br>Si tout va bien cliquer sur OK,<br>sinon appeler le support");
}

void sendModeConsole(Stream * stream, int mode)
{
    switch(mode){
        case 0 :
            addKeyValue(stream, STRAUTHID_MODE_CONSOLE, "forbid");
            break;
        case 1:
            addKeyValue(stream, STRAUTHID_MODE_CONSOLE, "allow");
            break;
        case 2:
            addKeyValue(stream, STRAUTHID_MODE_CONSOLE, "force");
            break;
    }
}

void authentication(Stream *stream)
{
    const char * ip = "10.10.14.71";
    const char * username = "QA\\Administrateur";
    const char * password = "S3cur3!1nux";
    const char * port = "3389";
    const char * protocol = "RDP";

    addKeyValue(stream, STRAUTHID_TARGET_DEVICE, ip);
    addKeyValue(stream, STRAUTHID_TARGET_USER, username);
    addKeyValue(stream, STRAUTHID_TARGET_PASSWORD, password);
    addKeyValue(stream, STRAUTHID_TARGET_PROTOCOL, protocol);
    addKeyValue(stream, STRAUTHID_TARGET_PORT, port);
    addKeyValue(stream, "INCONNU", "toto");
}

void recSession(Stream *stream)
{
    const char * video_quality = "low";
    const char * codec_id = "flv";

    addKeyValue(stream, STRAUTHID_OPT_MOVIE, "yes");
    addKeyValue(stream, STRAUTHID_OPT_MOVIE_PATH, "/tmp/test_rdp_movie.flv");
    addKeyValue(stream, STRAUTHID_VIDEO_QUALITY, video_quality);
    addKeyValue(stream, STRAUTHID_OPT_CODEC_ID, codec_id);
}

void recordingMessage(Stream *stream)
{
    const char * message = "Big brother is watching you<br>If you refused, you can not connect to remote host.<br>But you do what you want";
    addKeyValue(stream, STRAUTHID_MESSAGE, message);
    addKeyValue(stream, STRAUTHID_ACCEPT_MESSAGE, "ASK");
    addKeyValue(stream, STRAUTHID_TRANS_BUTTON_REFUSED, "refused");
}

void warningMessage(Stream *stream)
{
    const char * message = "Attention le mot de passe va expirer";
    addKeyValue(stream, STRAUTHID_MESSAGE, message);
    addKeyValue(stream, STRAUTHID_DISPLAY_MESSAGE, "ASK");
}

void closeConnection(Stream *stream)
{
    const char * endtime = "2293547500"; // passed timeframe, the server will disconnect immediately
    const char * timezone = "-3600";

    addKeyValue(stream, STRAUTHID_END_DATE_CNX, endtime);
    addKeyValue(stream, STRAUTHID_TIMEZONE, timezone);
}

void stopConnection(Stream *stream)
{
    const char * message = "Close connection";
    addKeyValue(stream, STRAUTHID_AUTHENTICATED, "False");
    addKeyValue(stream, STRAUTHID_REJECTED, message);
}

void keepalive(Stream *stream)
{
    addKeyValue(stream, STRAUTHID_KEEPALIVE, "True");
}


int hook(int sck)
{
    char buf[8192];
    char * p =buf;

    recv(sck, &buf, 4, 0);

    int packet_size = ntohl(*(uint32_t*)p);
    p+= 4;

    assert(packet_size < 8192);

    recv(sck, buf+4, packet_size-4, 0);

    buf[packet_size] = 0;

    // We should have a bunch of lines (keyword\nvalue\n)*
    enum { STATE_KEYWORD, STATE_VALUE } state = STATE_KEYWORD;
    char * keyword = buf + 4;
    char * value = buf + 4;
    for (char * p = buf + 4 ; p < buf + packet_size ; p++){
        switch (state){
        case STATE_KEYWORD:
            if (*p == '\n'){
                *p = 0;
                value = p+1;
                state = STATE_VALUE;
            }
        break;
        case STATE_VALUE:
            if (*p == '\n'){
                *p = 0;
                printf("%s: %s\n", keyword, value);
                checkKey(keyword, value);
                keyword = p+1;
                state = STATE_KEYWORD;
            }
        break;

        }
    }

    Stream stream(8192);
    uint8_t * hdr = stream.p;
    stream.p += 4;

    int mode_console = 0; /* 0 : forbid, 1 : allow, 2 : force */

    if (keepalived){
        keepalive(&stream);
    }
    else if (authenticated == 0){
        askAuthentication(&stream);
    }
    else if (warningMessageValidated==0){
        warningMessage(&stream);
    }
    else if (recordingMessageValidated==0){
        recordingMessage(&stream);
    }
    else if (recordingMessageValidated==2){
        stopConnection(&stream);
    }
    else {
        sendModeConsole(&stream, mode_console);
        recSession(&stream);
        closeConnection(&stream);
        authentication(&stream);
    }

    stream.mark_end();

    stream.p = hdr;
 ////    stream.out_uint32_be(0); /* version */

    int total_length = stream.end - stream.data;
    stream.out_uint32_be(total_length); /* size */

    send(sck, stream.data, total_length, 0);

    #warning should be replaced by a clean close, use transport object instead of socket
    sleep(2);
    for (;;)
        hook(sck);
    return 0;
}

int main()
{
    int sck = socket(AF_INET, SOCK_STREAM, 0);

    // reuse same port if a previous daemon was stopped
    int allow_reuse = 1;
    setsockopt(sck, SOL_SOCKET, SO_REUSEADDR, (char*)&allow_reuse, sizeof(allow_reuse));

    int port = 3350;
    const char *ip = "127.0.0.1";

    struct sockaddr_in saddr;

    memset(&saddr, 0, sizeof(struct sockaddr_in));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = inet_addr(ip);
    bind(sck, (struct sockaddr*)&saddr, sizeof(struct sockaddr_in));
    listen(sck, 2);

    struct pollfd fds[1];

    for (;;)
    {
        fds[0].fd = sck;
        fds[0].events = POLLIN;
        fds[0].revents = 0;
        switch(poll(fds, 1, 1000000)){
            case 0:
                printf("No event occured in 1000s, looping\n");
            break;
            // error
            case -1:
                switch(errno){
                    // An invalid file descriptor was given in one of the sets.
                    case EBADF:
                        printf("EBADF error\n");
                    break;
                    // The array given as argument was not contained in the
                    // calling program's address space.
                    case EFAULT:
                        printf("EFAULT error\n");
                    break;

                    // A signal occurred before any requested event; see signal(7).
                    case EINTR:
                        printf("EINTR error\n");
                    break;

                    // The nfds value exceeds the RLIMIT_NOFILE value.
                    case EINVAL:
                        printf("EINVAL error\n");
                    break;

                    // There was no space to allocate file descriptor tables.
                    case ENOMEM:
                        printf("ENOMEM error\n");
                    break;
                }
                exit(-1);
            break;
            // normal behavior, a positive number of events occured
            default:
                struct sockaddr_in sin;
                unsigned int sin_size = sizeof(struct sockaddr_in);
                memset(&sin, 0, sin_size);
                int in_sck = accept(sck, (struct sockaddr*)&sin, &sin_size);
                if (!fork()){
                    exit(hook(in_sck));
                }
                close(in_sck);
            break;
        }
    }
    return 0;
}
