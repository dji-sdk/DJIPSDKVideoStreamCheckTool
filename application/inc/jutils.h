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

#ifndef __J_UTILS_H__
#define __J_UTILS_H__

#include <iostream>
#include <pthread.h>

#include "jlog.h"

class JLock {
    public:
        JLock() {
            pthread_mutex_init(&mMutex, nullptr);
        }
        ~JLock() {
            pthread_mutex_destroy(&mMutex);
        }
        int lock() {
            pthread_mutex_lock(&mMutex);
            return 0;
        }
        int unlock() {
            pthread_mutex_unlock(&mMutex);
            return 0;
        }

    private:
        pthread_mutex_t mMutex;
};

#endif /* __J_UTILS_H__ */
