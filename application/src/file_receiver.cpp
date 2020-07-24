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

#include "file_receiver.h"
#include <string.h>

const uint32_t kBufferSize = 1024 * 1024;
const uint32_t kStreamMaxSize = MAX_J_STEAM_SIZE;
static const uint8_t kAudHex[] = {0x00, 0x00, 0x01, 0x09, 0x10};
static const uint8_t kAudHex2[] = {0x00, 0x00, 0x00, 0x01, 0x09, 0x10};
static const uint8_t kHeaderHex[] = {0x00, 0x00, 0x01};
static const uint8_t kHeaderHex2[] = {0x00, 0x00, 0x00, 0x01};

FileReceiver::FileReceiver(StreamBuffer *streamBuffer, char *filename)
    : ReceiverBase(streamBuffer),
      mBufferSize(kBufferSize),
      mTmpBuffer(nullptr),
      mFp(nullptr),
      mFilename(filename),
      mFilledSize(0)
{
    int ret = 0;

    if (!mFilename || mFilename[0] == '\0') {
        JLOGF("file name is NULL or name is empty, exit");
        exit(1);
    }

    mTmpBuffer = (uint8_t *) malloc(sizeof(uint8_t) * mBufferSize);
    if (!mTmpBuffer) {
        JLOGF("can't get buffer, exit");
        exit(1);
    }

    mFp = fopen(mFilename, "rb");
    if (!mFp) {
        JLOGF("open %s failed, exit", mFilename);
        exit(1);
    }
    JLOGI("init mFp %p or mTmpBuffer %p", mFp, mTmpBuffer);
}

FileReceiver::~FileReceiver()
{
    if (mFp) {
        fclose(mFp);
        mFp = nullptr;
    }
    if (mTmpBuffer) {
        free(mTmpBuffer);
        mTmpBuffer = nullptr;
    }
    JLOGI("deinit mFp %p or mTmpBuffer %p", mFp, mTmpBuffer);
}

int FileReceiver::recvStream(JBuffer *buffer)
{
    int ret = 0;
    while (!mFp || !mTmpBuffer || !buffer) {
        JLOGI("mFp %p or mTmpBuffer %p or buffer %p is NULL", mFp, mTmpBuffer, buffer);
        usleep(10 * 1000);
    }

    while (1) {
        /* to check mTmpBuffer if any data left */
        if (mFilledSize) {
            ret = getOneFrameStream();
        } else {
            /* need to read more buffer from file */
            ret = readStreamFromFile();
            if (ret) {
                JLOGU("Read stream end, stream TEST PASSED");
                exit(0);
            }
            ret = getOneFrameStream();
        }

        if (ret) {
            /* not get frameStream */
            if (mFilledSize >= kStreamMaxSize) {
                JLOGE("[Parse stream fail] maybe not have AUD nal or stream size is too long"
                      " Stream size should not more than %d bytes", kStreamMaxSize);
                exit(2);
            }
            /* need to read more buffer from file */
            ret = readStreamFromFile();
            if (ret) {
                JLOGR_PASSED("Congratulations, your stream has passed DJI's strict standards test.\n");
                exit(0);
            }
            continue;
        }

        if (mFoundEnd - mFoundBegin >= kStreamMaxSize) {
            JLOGE("[Parse stream fail] Stream size is too long, current size %d bytes, should no more than %d bytes",
                  mFoundEnd - mFoundBegin, kStreamMaxSize);
            exit(2);
        }

        /* Get 1 frame stream success, using AUD nal as the file */
        fillStreamBuffer(buffer);
//        JLOGU("Got a frame stream, stream size %u bytes", buffer->mFilledLen);

        moveTmpBuffer();
        return 0;
    }
}

int FileReceiver::getOneFrameStream()
{
    if (mFilledSize <= sizeof(kAudHex2))
        return 1;

    /* Find a stream end */
    for (int i = 0; i < mFilledSize - sizeof(kAudHex2); i++) {
#if 0 /* TODO */
        if ((memcmp(kAudHex, mTmpBuffer + i, sizeof(kAudHex)) == 0) ||
            (memcmp(kAudHex2, mTmpBuffer + i, sizeof(kAudHex2)) == 0)) {
        }
#endif
        if (memcmp(kAudHex2, mTmpBuffer + i, sizeof(kAudHex2)) == 0) {
            JLOGD("Got aud %d!!!!!", i);
            mFoundEnd = i + sizeof(kAudHex2);
            break;
        }
    }

    if (mFoundEnd == 0) {
        return 1;
    }

    /* Find a stream begin */
    for (int i = 0; i < mFoundEnd - sizeof(kHeaderHex2); i++) {
#if 0 /* TODO */
        if ((memcmp(kHeaderHex, mTmpBuffer + i, sizeof(kHeaderHex)) == 0) ||
            (memcmp(kHeaderHex2, mTmpBuffer + i, sizeof(kHeaderHex2)) == 0)) {
        }
#endif
        if (memcmp(kHeaderHex2, mTmpBuffer + i, sizeof(kHeaderHex2)) == 0) {
            JLOGD("Got header %d!!!!!", i);
            mFoundBegin = i;
            break;
        }
    }

    return 0;
}

int FileReceiver::readStreamFromFile()
{
    uint32_t readSize = mBufferSize - mFilledSize;
    int ret = 0;

    ret = fread(mTmpBuffer + mFilledSize, 1, readSize, mFp);
    if (ret != readSize) {
        /* Usually means reach the end of file */
        return 1;
    }

    mFilledSize += ret;

    return 0;
}

int FileReceiver::fillStreamBuffer(JBuffer *buffer)
{
    if (mFoundBegin) {
        JLOGE("%u bytes garbage data", mFoundBegin);
    }

    buffer->mFilledLen = mFoundEnd - mFoundBegin;
    buffer->mOffset = 0;
    memcpy(buffer->mData, mTmpBuffer + mFoundBegin, buffer->mFilledLen);

    return 0;
}

/* always keep buffer is from mTmpBuffer to mTmpBuffer + mFilledSize */
int FileReceiver::moveTmpBuffer()
{
    mFilledSize -= mFoundEnd;
    memmove(mTmpBuffer, mTmpBuffer + mFoundEnd, mFilledSize);
    /* TODO:  check other value could be clear */
    mFoundBegin = 0;
    mFoundEnd = 0;
}
