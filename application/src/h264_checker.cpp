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

#include "h264_checker.h"

const uint8_t ff_pict_type_to_slice_type[8] = {
    0xff,
    2,
    0,
    1,
    0xff,
    4,
    3,
    0xff
};

H264Checker::H264Checker(StreamBuffer *streamBuffer)
    : CheckerBase(streamBuffer)
{

    int ret = 0;
    mFrameCnt = 1;

    JLOGI("av_version_info is %s\n", av_version_info());
    JLOGI("avutil_configuration is \n%s\n", avutil_configuration());

#ifndef PROG_RELEASE
    av_log_set_level(AV_LOG_VERBOSE);
#endif

    /* create packet and frame */
    mPkt = av_packet_alloc();
    if (!mPkt) {
        JLOGF("av_packet_alloc error");
        exit(1);
    }

    mFrame = av_frame_alloc();
    if (!mFrame) {
        JLOGF("av_frame_alloc error");
        exit(1);
    }

    /* create avcodec and avparser */
    mCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!mCodec) {
        JLOGF("avcodec_find_decoder error");
        exit(1);
    }
    JLOGD("the decoder type is %s", mCodec->name);

    mParser = av_parser_init(mCodec->id);
    if (!mParser) {
        JLOGF("av_parser_init error");
        exit(1);
    }

    mCtx = avcodec_alloc_context3(mCodec);
    if (!mCtx) {
        JLOGF("avcodec_alloc_context3 error");
        exit(1);
    }

    /* GDR must used it */
    mCtx->flags2 |= AV_CODEC_FLAG2_SHOW_ALL;

    /* open it */
    if (avcodec_open2(mCtx, mCodec, NULL) < 0) {
        JLOGF("avcodec_open2 error");
        exit(1);
    }
    mParser->flags |= PARSER_FLAG_COMPLETE_FRAMES;

    JLOGI("ffmpeg resource init success\n");
}

H264Checker::~H264Checker()
{
    /* TODO */
    /* To make codec flush */
    //decode(mCtx, mFrame, NULL, outfilename);

    av_parser_close(mParser);
    avcodec_free_context(&mCtx);

    av_frame_free(&mFrame);
    av_packet_free(&mPkt);
}

int H264Checker::checkStream(JBuffer *buffer)
{
    int ret = 0;
    /* TODO: wait avcodec and avparser ctx is ready */
    if (!buffer || buffer->mFilledLen == 0) {
        JLOGF("buffer %p is NULL or filled len is empty", buffer);
        return 1;
    }

    /* parse and decode */
    mData = buffer->mData;
    mDataSize = buffer->mFilledLen;

    ret = av_parser_parse2(mParser, mCtx, &mPkt->data, &mPkt->size,
                           mData, mDataSize, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
    if (ret < 0) {
        JLOGF("av_parser_parse2 parsing error");
        exit(1);
    }

    if (mPkt->size) {
        ret = decode(buffer);
        if (ret) {
            JLOGE("decode error %d", ret);
        }
        JLOGI("buffer %p data %p filled len %u decoded frame count is %d type %d",
              buffer, buffer->mData, buffer->mFilledLen, mFrameCnt, mParser->pict_type);
    } else {
        JLOGE("parse pkt size empty buffer %p data %p filled len %u", buffer, buffer->mData, buffer->mFilledLen);
    }

    return 0;
}

int H264Checker::decode(JBuffer *buffer)
{
    char buf[1024];
    int ret = 0;

    ret = avcodec_send_packet(mCtx, mPkt);
    if (ret < 0) {
        JLOGF("Error sending a packet for decoding");
//        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(mCtx, mFrame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            //JLOGI("avcode_receive_frame return %d", ret);
            return 0;
        } else if (ret < 0) {
            JLOGF("Error during decoding");
            exit(1);
        }

        JLOGU("Decode frame done! Frame index:%4d", mCtx->frame_number);

        /* dump yuv */
#ifndef PROG_RELEASE
        if (mFrameCnt < 10) {
            JLOGI("dumping yuv frame cnt %d / %d", mFrameCnt, 10);
            snprintf(buf, sizeof(buf), "%s-%d", "yuv", mCtx->frame_number);
            pgm_save(mFrame->data[0], mFrame->linesize[0],
                     mFrame->width, mFrame->height, (char *)buf);
            pgm_save_u_v(mFrame->data[1], mFrame->width, mFrame->height, (char *)buf);
            pgm_save_u_v(mFrame->data[2], mFrame->width, mFrame->height, (char *)buf);
        }
#endif

        /* check frame */
        checkFrame();
        mFrameCnt++;
        buffer->mFilledLen = 0;
    }

    return 0;
}

int H264Checker::checkFrame()
{
    /* TODO: according to dji video stream stardard */
    SPS *sps = (SPS *) h264_get_sps_from_ctx(mCtx);
    PPS *pps = (PPS *) h264_get_pps_from_ctx(mCtx);
    int long_term_reference = h264_get_long_term_reference_from_ctx(mCtx);
    int max_num_reorder_frames = h264_get_max_num_reorder_frames(mCtx);
    int max_dec_frame_buffering = h264_get_max_dec_frame_buffering(mCtx);

//    JLOGE("******************************************\n"
//          "profile %d level %d mb_width %d mb_height %d \n"
//          "chroma_format_idc %d, bit_depth_luma %d bit_depth_chroma %d\n"
//          "scaling_matrix_present %d, frames_mbs_only_flag %d\n"
//          "slice_type %d, num_ref_idx_l0_default_active %d\n"
//          "num_slice_groups %d, max_num_ref_frames %d, num_reorder_frames %d\n"
//          "******************************************\n",
//          sps->profile_idc, sps->level_idc, sps->mb_width, sps->mb_height,
//          sps->chroma_format_idc, sps->bit_depth_luma, sps->bit_depth_chroma,
//          sps->scaling_matrix_present, sps->frame_mbs_only_flag,
//          mParser->pict_type, pps->ref_count[0],
//          pps->slice_group_count, sps->ref_frame_count, sps->num_reorder_frames
//    );

    frameCount++;

    if (mPkt->size > 256 * 1024) {
        JLOGR_FAILED("0.stream size is larger than 256K, reference 7.3.2.1.1");
    } else {
        totalCount++;
        JLOGR_PASSED("0.stream size(reference 7.3.2.1.1)");
    }

    /*! Stream standard 1: profile_idc */
    if (sps->profile_idc != 66 && sps->profile_idc != 77 && sps->profile_idc != 100) {
        JLOGR_FAILED("1.profile_idc %d, should be 66, 77 or 100, reference 7.3.2.1.1",
                     sps->profile_idc);
    } else {
        totalCount++;
        JLOGR_PASSED("1.profile_idc (reference 7.3.2.1.1)");
    }

    /*! Stream standard 2: level_idc */
    if (sps->level_idc > 51) {
        JLOGR_FAILED("2.level_idc %0.1f, should less than 5.1, reference 7.3.2.1.1",
                     sps->level_idc / 10.0);
    } else {
        totalCount++;
        JLOGR_PASSED("2.level_idc (reference 7.3.2.1.1)");
    }

    /*! Stream standard 3: chroma_format_idc */
    if (sps->chroma_format_idc != 1) {
        JLOGR_FAILED("3.chroma_format_idc %d, should be 1(YUV420), reference 7.3.2.1.1",
                     sps->chroma_format_idc);
    } else {
        totalCount++;
        JLOGR_PASSED("3.YUV420 chroma_format_idc (reference 7.3.2.1.1)");
    }

    /*! Stream standard 4: bit_depth_chroma */
    if (sps->bit_depth_chroma != 8 && sps->bit_depth_luma != 8) {
        JLOGR_FAILED("4.bit_depth_luma_minus8 %d bit_depth_chroma_minus8 %d"
                     ", should be 0(only 8-bit depth allowed), reference 7.3.2.1.1",
                     sps->bit_depth_luma - 8, sps->bit_depth_chroma - 8);
    } else {
        totalCount++;
        JLOGR_PASSED("4.chroma and luma only allow 8 bit (reference 7.3.2.1.1)");
    }

    /* TODO: pic??? seq_scaling_matrix_present is not allowed */
    /*! Stream standard 5: scaling_matrix_present */
    if (sps->scaling_matrix_present != 0) {
        JLOGR_FAILED("5.seq_scaling_matrix_presenst_flag %d"
                     ", should be 0, reference 7.3.2.1.1 and 7.3.2.2",
                     sps->scaling_matrix_present);
    } else {
        totalCount++;
        JLOGR_PASSED("5.seq_scaling_matrix_presenst_flag (reference 7.3.2.1.1 and 7.3.2.2)");
    }

    /*! Stream standard 6: frame_mbs_only_flag */
    if (sps->frame_mbs_only_flag != 1) {
        JLOGR_FAILED("6.frame_mbs_only_flag %d, should be 1, reference 7.3.2.1.1",
                     sps->frame_mbs_only_flag);
    } else {
        totalCount++;
        JLOGR_PASSED("6.frame_mbs_only_flag (reference 7.3.2.1.1)");
    }

    /*! Stream standard 7: slice type */
    if (ff_pict_type_to_slice_type[mParser->pict_type] != 0 &&
        ff_pict_type_to_slice_type[mParser->pict_type] != 2) {

        JLOGR_FAILED("[7.slice_type %d, should be 0 or 2, reference 7.3.3",
                     ff_pict_type_to_slice_type[mParser->pict_type]);
    } else {
        totalCount++;
        JLOGR_PASSED("7.slice_type (reference 7.3.3)");
    }

#if 0 /* TODO: could open it, if open it it means more strictly for ref_count check */
    /* num_ref_idx_l0_default_active_minus1 */
    if (pps->ref_count[0] != 1) {
        JLOGF("[Fail check stream] num_ref_idx_l0_default_active_minus1 %d, should be 0, reference 7.3.2.2",
                pps->ref_count[0] - 1);
        exit(100);
    }
#endif

    /*! Stream standard 8: slice_group_count */
    if (pps->slice_group_count != 1) {
        JLOGR_FAILED("8.num_slice_groups_minus1 %d, should be 0, reference 7.3.2.2",
                     pps->slice_group_count - 1);
    } else {
        totalCount++;
        JLOGR_PASSED("8.num_slice_groups_minus1 (reference 7.3.2.2)");
    }

    /*! Stream standard 9: ref_frame_count */
    if (sps->ref_frame_count != 1) {
        JLOGR_FAILED("9.max_num_ref_frames %d, should be 1, reference 7.3.2.1.1",
                     sps->ref_frame_count);
    } else {
        totalCount++;
        JLOGR_PASSED("9.max_num_ref_frames (reference 7.3.2.1.1)");
    }

//    /*! Stream standard 9: num_reorder_frames */
//    if (sps->num_reorder_frames != 5) {
//        JLOGR_FAILED("10.max_dec_frame_buffering %d, should be 5, reference 7.3.2.1.1",
//                     sps->num_reorder_frames);
//        exit(100);
//    } else {
//        JLOGR_PASSED("10.max_dec_frame_buffering (reference 7.3.2.1.1)");
//    }

    /* TODO: long_term_reference_flag */

#if 0
    /* TODO: max_num_reorder_frames */
    if (sps->num_reorder_frames != 1) {
        JLOGF("[Fail check stream] max_num_reorder_frames %d, should be 1, reference 7.3.2.1.1",
                sps->num_reorder_frames);
        exit(100);
    }
#endif

    /* TODO: max_dec_frame_buffering */
    /*! Stream standard 10: resolution limit */
    if (sps->mb_width > 120 || sps->mb_height > 68) {
        JLOGR_FAILED("10.width*height %dx%d, should less than 1920*1088, reference 7.3.2.1.1",
                     sps->mb_width * 16, sps->mb_height * 16);
    } else {
        totalCount++;
        JLOGR_PASSED("10.video resolution (reference 7.3.2.1.1)");
    }

    if (totalCount / 11 == frameCount) {
        framePassedCount++;
    }

    printf("\n");
    if (frameCount != framePassedCount) {
        JLOGR_FAILED("Currently, some frames failed the above standards test, frameCount:%d, totalCount:%d",
                     frameCount, framePassedCount);
    } else {
        JLOGR_PASSED("Currently, all frames has passed the above standards test.");
    }

    fflush(stdout);
    printf("\033[14A");
    printf("\033[K");

    return 0;
}

void H264Checker::pgm_save(unsigned char *buf, int wrap, int xsize, int ysize,
                           char *filename)
{
    FILE *f;
    int i;

    f = fopen(filename, "wb");
    for (i = 0; i < ysize; i++)
        fwrite(buf + i * wrap, 1, xsize, f);

    fclose(f);
}

void H264Checker::pgm_save_u_v(unsigned char *buf, int xsize, int ysize,
                               char *filename)
{
    FILE *f;

    f = fopen(filename, "ab");
    fwrite(buf, 1, xsize * ysize / 4, f);
    fclose(f);
}
