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

#ifndef __H264_CHECKER_H__
#define __H264_CHECKER_H__

#include "checker_base.h"

extern "C" {

#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include "h264_priv.h"

}

class H264Checker : public CheckerBase {
    public:
        H264Checker(StreamBuffer *streamBuffer);
        ~H264Checker();
        int checkStream(JBuffer *buffer);

    private:
        int decode(JBuffer *buffer);
        int checkFrame();
        void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize,
                     char *filename);
        void pgm_save_u_v(unsigned char *buf, int xsize, int ysize,
                     char *filename);

        const AVCodec *mCodec;
        AVCodecParserContext *mParser;
        AVCodecContext *mCtx;
        AVFrame *mFrame;
        AVPacket *mPkt;

        uint8_t *mData;
        size_t mDataSize;

        //H264Context *mH264Ctx; /* mH264Ctx->ps.sps */
        //SPS *mSps;

        int mFrameCnt;
};

#endif /* __H264_CHECKER_H__ */

