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

#ifndef __J_LOG_H__
#define __J_LOG_H__

#define MAX_J_STEAM_SIZE   (1024*1024)

#ifndef nullptr
#define nullptr NULL
#endif

#define PROG_RELEASE

#ifndef PROG_RELEASE
#define JLOGF(format,...)   printf("\033[35m[CHK-F] %s:%d " format "\033[0m\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define JLOGE(format,...)   printf("\033[31m[CHK-E] %s:%d " format "\033[0m\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define JLOGI(format,...)   printf("\033[32m[CHK-I] %s:%d " format "\033[0m\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define JLOGD(format,...)   printf("\033[33m[CHK-D] %s:%d " format "\033[0m\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define JLOGV(format,...)   printf("\033[37m[CHK-V] %s:%d " format "\033[0m\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define JLOGF(format,...)   printf("\033[35m[Fail   ] " format "\033[0m\n", ##__VA_ARGS__)
#define JLOGE(format,...)   printf("\033[31m[Error  ] " format "\033[0m\n", ##__VA_ARGS__)
#define JLOGI(format,...)
#define JLOGD(format,...)
#define JLOGV(format,...)
#endif

/* Log for user need */
#define JLOGU(format, ...)   printf("\033[33m[Decoding] " format "\033[0m\n", ##__VA_ARGS__)

#define JLOGR_PASSED(format, ...)   printf("\033[32m[Passed] " "\033[0m" format "\n", ##__VA_ARGS__)
#define JLOGR_FAILED(format, ...)   printf("\033[31m[Failed] " "\033[0m" format "\n", ##__VA_ARGS__)

#endif /* __J_LOG_H__ */
