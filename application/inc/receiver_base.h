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

#ifndef __RECIEVER_BASE_H__
#define __RECIEVER_BASE_H__

#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <pthread.h>

#include "jlog.h"
#include "stream_buffer.h"

class ReceiverBase {
    public:
        ReceiverBase(StreamBuffer *streamBuffer);
        ~ReceiverBase();

        virtual int recvStream(JBuffer *buffer);
        void threadRun();

        /* This buffer is allocate outsize, just a pointer to use it */
        StreamBuffer *mStreamBuffer;
    private:
        bool mIsStop;
        pthread_t mThread;
};

#endif /* __RECIEVER_BASE_H__ */
