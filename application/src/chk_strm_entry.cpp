/**
 ********************************************************************
 * @file
 *
 * @copyright (c) 2017-2020 DJI. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *
 * This program is also available under a commercial proprietary license.
 *********************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "checker_base.h"
#include "h264_checker.h"
#include "receiver_base.h"
#include "file_receiver.h"
#include "udp_receiver.h"
#include "stream_buffer.h"

#define TOOL_VERSION_MAJOR     1
#define TOOL_VERSION_MINOR     0
#define TOOL_VERSION_MODIFY    0
#define TOOL_VERSION_BUILD     0

typedef struct chk_info_t {
    char *filename;
    int mode;
    int port;
    StreamBuffer *stream;
    CheckerBase *checker;
    ReceiverBase *receiver;
} chk_info_t, *chk_info_handle_t;

void usage()
{
    fprintf(stderr, "\ndji_check_stream usage:\n");
    fprintf(stderr, "./dji_check_stream <-u [-p <port>] |-f -i <filename>> [-h]\n");
    fprintf(stderr, "sample:\n");
    fprintf(stderr, "case 1: using a udp receive, a frame stream should end up with an AUD\n");
    fprintf(stderr, "   dji_check_stream -u -p udp_port\n");
    fprintf(stderr, "       udp_port is the one you sendto default is 45654\n\n");
    fprintf(stderr, "case 2: using a file reader\n");
    fprintf(stderr, "       dji_check_stream -f -i filename\n");
    fprintf(stderr, "           filename should in ascii charactor path, could not be ignored\n");

    exit(1);
}

void version()
{
    fprintf(stderr, "\ndji_check_stream Version:V%02d.%02d.%02d.%02d\n",
            TOOL_VERSION_MAJOR,
            TOOL_VERSION_MINOR,
            TOOL_VERSION_MODIFY,
            TOOL_VERSION_BUILD);

    exit(1);
}

#define MODE_INVALD         0
#define MODE_UDP_RECEIVER   1
#define MODE_FILE_READER    2

int main(int argc, char *argv[])
{
    int ch;
    chk_info_t chk_info_ctx;
    chk_info_handle_t chk_info = &chk_info_ctx;

    memset(chk_info, 0, sizeof(chk_info_t));
    chk_info->port = 45654;

    while((ch = getopt(argc, argv, "up:fi:h")) != -1) {
        switch(ch) {
        case 'u':
            chk_info->mode = MODE_UDP_RECEIVER;
            break;
        case 'p':
            chk_info->port = atoi(optarg);
            break;
        case 'f':
            chk_info->mode = MODE_FILE_READER;
            break;
        case 'i':
            chk_info->filename = strdup(optarg);
            break;
        case 'h':
            usage();
                break;
            case 'v':
                break;
            default:
            fprintf(stderr, "option %c mismatched\n", ch);
            usage();
            break;
        }
    }

    switch (chk_info->mode) {
    case MODE_UDP_RECEIVER:
        /* TODO: */
        fprintf(stderr, "udp reader mode, port %d\n", chk_info->port);
        chk_info->stream = new StreamBuffer(MAX_J_STEAM_SIZE, 10);
        chk_info->checker = new H264Checker(chk_info->stream);
        chk_info->receiver = new UdpReceiver(chk_info->stream, chk_info->port);
        break;
    case MODE_FILE_READER:
        if (!chk_info->filename) {
            fprintf(stderr, "file reader mode, filename must be filled\n");
            usage();
            exit(1);
        }
        chk_info->stream = new StreamBuffer(MAX_J_STEAM_SIZE, 10);
        chk_info->checker = new H264Checker(chk_info->stream);
        chk_info->receiver = new FileReceiver(chk_info->stream, chk_info->filename);
        break;
    default:
        fprintf(stderr, "invalid parameters\n");
        usage();
        exit(1);
    }

    while (1) {
        sleep(1);
    }

    return 0;
}
