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

#include "stream_buffer.h"

StreamBuffer::StreamBuffer(uint32_t size, uint32_t number) {
    mBufferSize = size;
    mBufferNumber = number;

    makeBuffer();
}

StreamBuffer::~StreamBuffer() {
    /* TODO */
}

int StreamBuffer::makeBuffer() {
    for (int i = 0; i < mBufferNumber; i++) {
        JLOGD("Allocating buffer %d / %u size %u", i, mBufferNumber, mBufferSize);
        JBuffer *buffer = nullptr;
        buffer = new JBuffer(mBufferSize);
        JLOGD("Allocated buffer %d / %u size %u data %p", i, mBufferNumber, mBufferSize,
                buffer->mData);
        mEmptyLock.lock();
        mEmptyQ.push(buffer);
        mEmptyLock.unlock();
    }

    return 0;
}

JBuffer *StreamBuffer::getEmptyBuffer() {
    JBuffer *buffer = nullptr;

    mEmptyLock.lock();
    if (mEmptyQ.empty()) {
        JLOGD("emptyQ is empty now, no buffer to get");
        mEmptyLock.unlock();
        return buffer;
    }

    buffer = mEmptyQ.front();
    JLOGD("Get buffer from emptyQ, buffer %p data %p size %u", buffer, buffer->mData,
            buffer->mSize);
    mEmptyQ.pop();
    mEmptyLock.unlock();

    return buffer;
}

int StreamBuffer::putEmptyBuffer(JBuffer *buffer) {

    if (!buffer) {
        JLOGE("Null buffer input, return");
        return 1;
    }

    JLOGD("Push buffer to emptyQ, buffer %p data %p size %u", buffer, buffer->mData,
            buffer->mSize);
    mEmptyLock.lock();
    mEmptyQ.push(buffer);
    mEmptyLock.unlock();

    return 0;
}

JBuffer *StreamBuffer::getFilledBuffer() {
    JBuffer *buffer = nullptr;

    mFilledLock.lock();
    if (mFilledQ.empty()) {
        JLOGD("filledQ is empty now, no buffer to get");
        mFilledLock.unlock();
        return buffer;
    }

    buffer = mFilledQ.front();
    JLOGD("Get buffer from emptyQ, buffer %p data %p size %u", buffer, buffer->mData,
            buffer->mSize);
    mFilledQ.pop();
    mFilledLock.unlock();

    return buffer;
}

int StreamBuffer::putFilledBuffer(JBuffer *buffer) {
    if (!buffer) {
        JLOGE("Null buffer input, return");
        return 1;
    }

    JLOGD("Push buffer to emptyQ, buffer %p data %p size %u", buffer, buffer->mData,
            buffer->mSize);
    mFilledLock.lock();
    mFilledQ.push(buffer);
    mFilledLock.unlock();

    return 0;
}
