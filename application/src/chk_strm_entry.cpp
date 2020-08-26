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

#define TOOL_VERSION_MAJOR             1
#define TOOL_VERSION_MINOR             0
#define TOOL_VERSION_MODIFY            0

#define MODE_INVALD                    0
#define MODE_UDP_RECEIVER              1
#define MODE_FILE_READER               2

#define VIDEO_STREAM_TYPE_INVALID      0
#define VIDEO_STREAM_TYPE_DJI_H264     1
#define VIDEO_STREAM_TYPE_CUSTOM_H264  2

#define VIDEO_FRAME_MAX_COUNT          (18000)
#define VIDEO_FRAME_AUD_LEN            (6)

static const uint8_t s_frameAudInfo[VIDEO_FRAME_AUD_LEN] = {0x00, 0x00, 0x00, 0x01, 0x09, 0x10};

typedef struct chk_info_t {
    char *filename;
    int mode;
    int port;
    int type;
    StreamBuffer *stream;
    CheckerBase *checker;
    ReceiverBase *receiver;
} chk_info_t, *chk_info_handle_t;

typedef struct {
    float durationS;
    uint32_t positionInFile;
    uint32_t size;
} video_frame_info_t;

void usage()
{
    fprintf(stderr, "\nstream_check_tool usage:\n");
    fprintf(stderr, "./stream_check_tool <-u [-p <port>] [-t <type>] |-f [-i <filename>] [-t <type>] > [-h] [-v]\n");
    fprintf(stderr, "\n-t type: Stream type, can be any value of \"DJI-H264\" or \"Custom-H264\"\n\n");
    fprintf(stderr, "sample:\n");
    fprintf(stderr,
            "case 1: video stream received by UDP transmission channel as input, and please ensure every frame end with AUD (0x00 0x00 0x00 0x01 0x09 0x10)\n");
    fprintf(stderr, "   stream_check_tool -u -p udp_port -t stream_type\n");
    fprintf(stderr, "       udp_port is the UDP port which send video stream to, and the default value is 45654\n\n");
    fprintf(stderr, "case 2: video file as input\n");
    fprintf(stderr, "       stream_check_tool -f -i filename -t stream_type\n");

    exit(1);
}

void version()
{
    fprintf(stderr, "\ndji_check_stream Version:V%02d.%02d.%02d\n",
            TOOL_VERSION_MAJOR,
            TOOL_VERSION_MINOR,
            TOOL_VERSION_MODIFY);

    exit(1);
}

int32_t getH264Nalu(FILE *pFile, uint8_t *pNalu, uint32_t *packetLength, uint32_t *filePosition)
{
    int32_t pos = 0;
    int32_t len = 0;

    if (pFile == NULL) {
        return -1;
    }

    if (fread(pNalu, 1, 4, pFile) <= 0) {
        return -1;
    }

    if (pNalu[0] != 0 || pNalu[1] != 0 || pNalu[2] != 0 || pNalu[3] != 1) {
        return -1;
    }

    pos = 4;

    while (1) {
        if (feof(pFile))
            break;

        pNalu[pos] = fgetc(pFile);

        if (pNalu[pos - 3] == 0 && pNalu[pos - 2] == 0 && pNalu[pos - 1] == 0 && pNalu[pos] == 1) {
            fseek(pFile, -4, SEEK_CUR);
            pos -= 4;
            break;
        }

        pos++;
    }

    len = pos + 1;

    *packetLength = len;
    *filePosition = ftell(pFile);

    return len;
}

int32_t getFrameInfoOfVideoFile(const char *path, video_frame_info_t *frameInfo, uint32_t frameInfoBufferCount,
                                uint32_t *frameCount)
{
    FILE *pFile = NULL;
    int32_t len;
    unsigned char *pBuf = NULL;
    unsigned char *pNalu = NULL;
    unsigned char naluType;
    bool addSlice = false;
    uint32_t spsLen = 0;
    uint32_t ppsLen = 0;
    uint32_t sliceLen = 0;
    uint32_t packetCount = 0;
    uint32_t length = 0;
    uint32_t position = 0;

    pFile = fopen(path, "rb");
    if (pFile == NULL) {
        printf("Open file error");
        return -1;
    }

    pBuf = (unsigned char *) malloc(1024 * 1024);
    if (pBuf == NULL) {
        printf("Malloc error");
        return -1;
    }

    while (1) {
        len = getH264Nalu(pFile, pBuf, &length, &position);
        if (len <= 0)
            break;

        if (pBuf[0] != 0 || pBuf[1] != 0 || pBuf[2] != 0 || pBuf[3] != 1)
            continue;

        pNalu = pBuf + 4;
        naluType = pNalu[0] & 0x1F;

        switch (naluType) {
            case 0x07: // SPS
                spsLen = len;
                break;
            case 0x08: // PPS
                ppsLen = len;
                addSlice = true;
                break;
            default:
                if (addSlice == true) {
                    sliceLen = len + spsLen + ppsLen;
                    addSlice = false;
                } else {
                    sliceLen = len;
                }
                frameInfo[packetCount].size = sliceLen;
                frameInfo[packetCount].positionInFile = position - frameInfo[packetCount].size;
                *frameCount = packetCount;

                if (packetCount >= frameInfoBufferCount) {
                    printf("frame buffer is full.");
                    goto out;
                }
                packetCount++;
                break;
        }
    }

out:
    free(pBuf);
    fclose(pFile);

    return 0;
}

void addAudInfoForCustomH264(const char *filename)
{
    FILE *fpFile = NULL;
    FILE *fpFileOut = NULL;
    uint32_t frameCount = 0;
    video_frame_info_t *frameInfo = NULL;
    uint8_t *dataBuffer;
    uint32_t dataLength = 0;
    int32_t ret = 0;

    frameInfo = (video_frame_info_t *) malloc(
        VIDEO_FRAME_MAX_COUNT * sizeof(video_frame_info_t));
    if (frameInfo == NULL) {
        printf("Malloc error");
        exit(1);
    }

    fpFile = fopen(filename, "rb+");
    if (fpFile == NULL) {
        printf("Open video file fail");
        sleep(1);
    }

    fpFileOut = fopen("tmp.h264", "wb+");
    if (fpFileOut == NULL) {
        exit(1);
    }

    getFrameInfoOfVideoFile(filename, frameInfo, VIDEO_FRAME_MAX_COUNT, &frameCount);

    uint32_t i = 0;
    for (i = 0; i < frameCount; i++) {
        dataBuffer = (uint8_t *) calloc(frameInfo[i].size + VIDEO_FRAME_AUD_LEN, 1);
        if (dataBuffer == NULL) {
            printf("Malloc fail");
        }

        ret = fseek(fpFile, frameInfo[i].positionInFile, SEEK_SET);
        if (ret != 0) {
            printf("Seek file fail");
        }

        dataLength = fread(dataBuffer, 1, frameInfo[i].size, fpFile);
        memcpy(&dataBuffer[frameInfo[i].size], s_frameAudInfo, VIDEO_FRAME_AUD_LEN);
        dataLength = dataLength + VIDEO_FRAME_AUD_LEN;
        fwrite(dataBuffer, 1, dataLength, fpFileOut);
        free(dataBuffer);
    }
}

int main(int argc, char *argv[])
{
    int ch;
    chk_info_t chk_info_ctx;
    chk_info_handle_t chk_info = &chk_info_ctx;

    memset(chk_info, 0, sizeof(chk_info_t));
    chk_info->port = 45654;

    while ((ch = getopt(argc, argv, "up:fi:hvt:")) != -1) {
        switch (ch) {
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
                version();
                break;
            case 't':
                if (strcmp(optarg, "Custom-H264") == 0) {
                    chk_info->type = VIDEO_STREAM_TYPE_CUSTOM_H264;
                } else if (strcmp(optarg, "DJI-H264") == 0) {
                    chk_info->type = VIDEO_STREAM_TYPE_DJI_H264;
                } else {
                    fprintf(stderr, "invalid video stream type:%s\n", strdup(optarg));
                    exit(0);
                }
                break;
            default:
                fprintf(stderr, "option %c mismatched\n", ch);
                usage();
                break;
        }
    }

    if (VIDEO_STREAM_TYPE_INVALID == chk_info->type) {
        fprintf(stderr, "please input the video stream type, such as DJI-H264 or Custom-H264\n\n");
        exit(0);
    }

    switch (chk_info->mode) {
        case MODE_UDP_RECEIVER:
            fprintf(stderr, "udp reader mode, port %d\n", chk_info->port);
            chk_info->stream = new StreamBuffer(MAX_J_STEAM_SIZE, 10);
            chk_info->checker = new H264Checker(chk_info->stream);
            if (chk_info->type == VIDEO_STREAM_TYPE_CUSTOM_H264) {
                fprintf(stderr, "udp receive mode not support this video stream format\n");
                exit(1);
            } else {
                chk_info->receiver = new UdpReceiver(chk_info->stream, chk_info->port);
            }
            break;
        case MODE_FILE_READER:
            if (!chk_info->filename) {
                fprintf(stderr, "file reader mode, filename must be filled\n");
                usage();
                exit(1);
            }
            chk_info->stream = new StreamBuffer(MAX_J_STEAM_SIZE, 10);
            chk_info->checker = new H264Checker(chk_info->stream);
            if (chk_info->type == VIDEO_STREAM_TYPE_CUSTOM_H264) {
                addAudInfoForCustomH264(chk_info->filename);
                chk_info->receiver = new FileReceiver(chk_info->stream, (char *) "tmp.h264");
            } else {
                chk_info->receiver = new FileReceiver(chk_info->stream, chk_info->filename);
            }
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
