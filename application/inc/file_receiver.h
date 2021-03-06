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

#ifndef _FILE_RECEIVER_H__
#define _FILE_RECEIVER_H__

#include "receiver_base.h"

class FileReceiver : public ReceiverBase {
    public:
        FileReceiver(StreamBuffer *streamBuffer, char *filename);
        ~FileReceiver();
        int recvStream(JBuffer *buffer);

    private:
        uint32_t mBufferSize;
        uint8_t *mTmpBuffer;
        FILE *mFp;
        char *mFilename;

        uint32_t mFilledSize;
        //uint32_t mCurPos;

        uint32_t mFoundBegin;
        uint32_t mFoundEnd;

        int getOneFrameStream();
        int readStreamFromFile();
        int fillStreamBuffer(JBuffer *buffer);
        int moveTmpBuffer();
};

#endif /* _FILE_RECEIVER_H__ */
