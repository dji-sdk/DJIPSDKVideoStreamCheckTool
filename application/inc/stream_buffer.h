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

#ifndef __STREAM_BUFFER_H__
#define __STREAM_BUFFER_H__

#include <iostream>
#include <queue>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "jlog.h"
#include "jutils.h"

class JBuffer {
    public:
        JBuffer(uint32_t size) {
            mFilledLen = 0;
            mOffset = 0;
            mSize = size; 
            mData = (uint8_t *)calloc(1, sizeof(uint8_t) * size); 
            if (!mData) {
                JLOGE("malloc buffer failed");
                mData = nullptr;
                mSize = 0;
            }
            JLOGD("Allocate buffer %p size %u", mData, mSize);
        }
        ~JBuffer() {
            if (mData) {
                JLOGD("Free buffer %p size %u", mData, mSize);
                free(mData);
                mData = nullptr;
            }
        }

        uint8_t *mData;
        uint32_t mSize;
        uint32_t mFilledLen;
        uint32_t mOffset;
};

class StreamBuffer {
    private:
        int makeBuffer();
    public:
        StreamBuffer(uint32_t size, uint32_t number = 10);
        ~StreamBuffer();
        JBuffer *getEmptyBuffer();
        int putEmptyBuffer(JBuffer *buffer);
        JBuffer *getFilledBuffer();
        int putFilledBuffer(JBuffer *buffer);

        /* mutex lock to protect emptyQueue and filledQueue */
        std::queue<JBuffer *> mEmptyQ;
        JLock mEmptyLock;
        std::queue<JBuffer *> mFilledQ;
        JLock mFilledLock;

        uint32_t mBufferSize;
        uint32_t mBufferNumber;
};

#endif /* __STREAM_BUFFER_H__ */
