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

#ifndef __UDP_RECEIVER_H__
#define __UDP_RECEIVER_H__

#include "receiver_base.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class UdpReceiver : public ReceiverBase {
    public:
        UdpReceiver(StreamBuffer *streamBuffer, int port);
        ~UdpReceiver();
        int recvStream(JBuffer *buffer);

    private:
        int mPort;
        int mSockFd;

        struct sockaddr_in mSerAddr;
        volatile bool mInited;

        bool haveAud(JBuffer *buffer);
        void adjustStreamOffset(JBuffer *buffer);
};

#endif /* __UDP_RECEIVER_H__ */
