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

#include "checker_base.h"
#include <sys/prctl.h>

static void *threadEntry(void *user_data) {
    CheckerBase *my = (CheckerBase *)user_data;

    if (!my) {
        JLOGE("threadRun user_data is NULL");
        return nullptr;
    }

    my->threadRun();
    return nullptr;
}

CheckerBase::CheckerBase(StreamBuffer *streamBuffer)
    : mStreamBuffer(streamBuffer), mIsStop(false) {
    int ret = 0;
    pthread_attr_t attr;

    /* create a thread */
    ret = pthread_attr_init(&attr);
    if (ret) {
        JLOGE("pthread_attr_init error %d", ret);
        exit(1);
    }
    ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    if (ret) {
        JLOGE("pthread_attr_setdetachstate joinable error %d", ret);
        exit(1);
    }
    pthread_attr_destroy(&attr);

    ret = pthread_create(&mThread, &attr, threadEntry, (void *)this);
    if (ret) {
        JLOGE("pthread_create error %d", ret);
        exit(1);
    }
    JLOGD("thread create success");
}

CheckerBase::~CheckerBase() {
    mIsStop = true;
    pthread_join(mThread, NULL);
    JLOGD("thread destory success");
}

/* The real recvStream program should be implenmented in the derived-class */
int CheckerBase::checkStream(JBuffer *buffer) {
    if (!buffer) {
        JLOGE("input buffer is null, return");
        return 1;
    }

    JLOGI("base checkStream buffer %p data %p size %u/%u", buffer, buffer->mData,
            buffer->mFilledLen, buffer->mSize);
    buffer->mFilledLen = 0;
    usleep(50*1000);

    return 0;
}

void CheckerBase::threadRun() {
    JBuffer *buffer = nullptr;
    int ret = 0;

    prctl(PR_SET_NAME, "check");

    if (!mStreamBuffer) {
        JLOGE("stream buffer is NULL");
        return;
    }

    while (!mIsStop) {
        JLOGV("thread %p is running", this);

        /* wait until get empty buffer */
        buffer = mStreamBuffer->getFilledBuffer();
        if (!buffer) {
            JLOGD("didn't get filled stream buffer, continue");
            usleep(100*1000);
            continue;
        }

        /* receive stream into buffer, until single complete stream is got */
        ret = checkStream(buffer);
        if (ret) {
            JLOGE("checkStream error %d, push buffer back", ret);
            buffer->mFilledLen = 0;
            mStreamBuffer->putEmptyBuffer(buffer);
            continue;
        }

        buffer->mFilledLen = 0;

        JLOGD("checkStream success, buffer %p data %p", buffer, buffer->mData);

        /* put buffer into filled buffer queue */
        ret = mStreamBuffer->putEmptyBuffer(buffer);
        if (ret) {
            JLOGF("putEmptyBuffer error %d, push buffer back", ret);
            mStreamBuffer->putEmptyBuffer(buffer);
            continue;
        }
    }
}

