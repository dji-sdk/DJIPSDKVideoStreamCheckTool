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

#ifndef __DEFINE_H__
#ifndef __DEFINE_H__

typedef uint16_t            sc_result_t;
#define SUCCESS             0
#define ERR_FAILURE         -8001
#define ERR_TIMEOUT         -8002
#define ERR_PARAM           -8003
#define ERR_NO_MEM          -8005
#define ERR_MISMATCH        -8007

#define LOGI(format,...)  printf(format "\n", ##__VA_ARGS__)
#define LOGE(format,...)  printf(format "\n", ##__VA_ARGS__)

#define SC_CHECK_NULL(x)    \
    do {                    \
        if (!x) {           \
            LOGE("%s is NULL", #x); \
            return ERR_PARAM;   \
        }\
    } while (0)

#endif /* !__DEFINE_H__ */
