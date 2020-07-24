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

#include "udp_receiver.h"

#include <string.h>

static const uint8_t kAudHex[] = {0x00, 0x00, 0x01, 0x09, 0x10};
static const uint8_t kAudHex2[] = {0x00, 0x00, 0x00, 0x01, 0x09, 0x10};
static const uint8_t kHeaderHex[] = {0x00, 0x00, 0x01};
static const uint8_t kHeaderHex2[] = {0x00, 0x00, 0x00, 0x01};

UdpReceiver::UdpReceiver(StreamBuffer *streamBuffer, int port)
    : ReceiverBase(streamBuffer),
      mPort(port) {

    mInited = false;

    int ret = 0;
    mSockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (mSockFd < 0) {
        JLOGF("open socket failed");
        exit(1);
    }

    memset(&mSerAddr, 0, sizeof(mSerAddr));
    mSerAddr.sin_family = AF_INET;
    mSerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    mSerAddr.sin_port = htons(mPort);

    ret = bind(mSockFd, (struct sockaddr*)&mSerAddr, sizeof(mSerAddr));
    if(ret < 0)
    {
        JLOGF("socket bind fail, port %d!\n", mPort);
        exit(1);
    }

    mInited = true;
}

UdpReceiver::~UdpReceiver() {
    if (mSockFd) {
        close(mSockFd);
        mSockFd = 0;
    }
}

bool UdpReceiver::haveAud(JBuffer *buffer) {
    if (buffer->mFilledLen < sizeof(kAudHex2)) {
        JLOGI("stream length is too short");
        return false;
    }

    /* FIXME: maybe need to parse the stream from the header */
    if (memcmp(kAudHex2, buffer->mData + buffer->mFilledLen - sizeof(kAudHex2),
                sizeof(kAudHex2)) == 0 ||
        memcmp(kAudHex, buffer->mData + buffer->mFilledLen - sizeof(kAudHex),
                sizeof(kAudHex)) == 0) {

        JLOGD("Got aud!!!!!");
        return true;
    }

    return false;
}

void UdpReceiver::adjustStreamOffset(JBuffer *buffer) {
    int headerPos = 0;

    for (int i = 0; i < buffer->mFilledLen - sizeof(kHeaderHex2); i++) {
#if 0 /* TODO */
        if ((memcmp(kHeaderHex, mTmpBuffer + i, sizeof(kHeaderHex)) == 0) ||
            (memcmp(kHeaderHex2, mTmpBuffer + i, sizeof(kHeaderHex2)) == 0)) {
        }
#else
        if (memcmp(kHeaderHex2, buffer->mData + i, sizeof(kHeaderHex2)) == 0) {
#endif
            JLOGD("Got header %d!!!!!", i);
            headerPos = i;
            break;
        }
    }

    if (headerPos) {
        JLOGD("MOVE buffer, pos %d", headerPos);
        memmove(buffer->mData, buffer->mData + headerPos, buffer->mFilledLen - headerPos);
        buffer->mFilledLen -= headerPos;
    }
}

int UdpReceiver::recvStream(JBuffer *buffer) {

    int ret = 0;
    socklen_t sockLen;
    struct sockaddr_in clientAddr;
    int offset = 0;

    while (!mInited) {
        JLOGI("socket not inited yet, wait");
        usleep(10*1000);
    }

    while (1) {
        ret = recvfrom(mSockFd, buffer->mData + offset, buffer->mSize - offset, 0,
                (struct sockaddr *)&clientAddr, &sockLen);
        if (ret < 0) {
            /* TODO: */
            JLOGF("udp receive error, ret %d", ret);
            exit(1);
        }
        JLOGI("recv a packet len %d", ret);
        offset += ret;
        buffer->mFilledLen = offset;

        /* check stream is alread end or not */
        if (haveAud(buffer)) {
            /* check h264 header, make sure h264 header is in the beginning of the stream */
            adjustStreamOffset(buffer);
            JLOGI("Got a complete frame stream, stream size %d", buffer->mFilledLen);
            return 0;
        }

        /* Check size too long or not */
        if (offset >= buffer->mSize) {
            JLOGE("[Parse stream fail] maybe not have AUD nal or stream size is too long"
                    " Stream size should not more than %d bytes", MAX_J_STEAM_SIZE);
            exit(1);
        }
        /* Buffer not complete, should receiver again */
    }
}
