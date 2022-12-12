/******************************************************************************
  Copyright (C), 2001-2016, Allwinner Tech. Co., Ltd.
 ******************************************************************************
  File Name     :
  Version       : Initial Draft
  Author        : Allwinner BU3-PD2 Team
  Created       : 2016/11/4
  Last Modified :
  Description   :
  Function List :
  History       :
******************************************************************************/

//#define LOG_NDEBUG 0
#define LOG_TAG "SampleVencRecreate"

#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "plat_log.h"
#include <mm_common.h>
#include <mpi_videoformat_conversion.h>
#include <mpi_region.h>
#include <mpi_vi_private.h>
#include "sample_vencRecreate.h"
#include "sample_vencRecreate_config.h"

#define ISP_RUN (1)

static SAMPLE_VENC_RECREATE_S *gpVirvi2VencData = NULL;

static void handle_exit(int signo)
{
    alogd("user want to exit!");
    if(NULL != gpVirvi2VencData)
    {
        cdx_sem_up(&gpVirvi2VencData->mSemExit);
    }
}

static ERRORTYPE InitVirvi2VencData(SAMPLE_VENC_RECREATE_S *pContext)
{
    if (pContext == NULL)
    {
        aloge("malloc struct fail");
        return FAILURE;
    }
    memset(pContext, 0, sizeof(SAMPLE_VENC_RECREATE_S));
    pContext->mVeChn = MM_INVALID_CHN;
    pContext->mViChn = MM_INVALID_CHN;
    pContext->mViDev = MM_INVALID_DEV;

    return SUCCESS;
}

static ERRORTYPE parseCmdLine(SAMPLE_VENC_RECREATE_S *pContext, int argc, char** argv)
{
    ERRORTYPE ret = FAILURE;

    if(argc <= 1)
    {
        alogd("use default config.");
        return SUCCESS;
    }
    while (*argv)
    {
       if (!strcmp(*argv, "-path"))
       {
          argv++;
          if (*argv)
          {
              ret = SUCCESS;
              if (strlen(*argv) >= MAX_FILE_PATH_LEN)
              {
                 aloge("fatal error! file path[%s] too long:!", *argv);
              }

              strncpy(pContext->mCmdLinePara.mConfigFilePath, *argv, MAX_FILE_PATH_LEN-1);
              pContext->mCmdLinePara.mConfigFilePath[MAX_FILE_PATH_LEN-1] = '\0';
          }
       }
       else if(!strcmp(*argv, "-h"))
       {
            printf("CmdLine param:\n"
                "\t-path /home/sample_vi2venc2muxer.conf\n");
            break;
       }
       else if (*argv)
       {
          argv++;
       }
    }

    return ret;
}

static ERRORTYPE loadConfigPara(SAMPLE_VENC_RECREATE_S *pContext, const char *conf_path)
{
    int ret = 0;
    char *ptr = NULL;

    if (conf_path != NULL)
    {
        CONFPARSER_S mConf;
        memset(&mConf, 0, sizeof(CONFPARSER_S));
        ret = createConfParser(conf_path, &mConf);
        if (ret < 0)
        {
            aloge("load conf fail");
            return FAILURE;
        }

        pContext->mConfigPara.mVippDev = GetConfParaInt(&mConf, CFG_VIPP_DEV_ID, 0);
        pContext->mConfigPara.mVeChn = GetConfParaInt(&mConf, CFG_VENC_CH_ID, 0);
        alogd("vippDev: %d, veChn: %d", pContext->mConfigPara.mVippDev, pContext->mConfigPara.mVeChn);

        pContext->mConfigPara.srcWidth = GetConfParaInt(&mConf, CFG_SRC_WIDTH, 0);
        pContext->mConfigPara.srcHeight = GetConfParaInt(&mConf, CFG_SRC_HEIGHT, 0);
        alogd("srcWidth: %d, srcHeight: %d", pContext->mConfigPara.srcWidth, pContext->mConfigPara.srcHeight);

        pContext->mConfigPara.dstWidth = GetConfParaInt(&mConf, CFG_DST_VIDEO_WIDTH, 0);
        pContext->mConfigPara.dstHeight = GetConfParaInt(&mConf, CFG_DST_VIDEO_HEIGHT, 0);
        alogd("dstWidth: %d, dstHeight: %d", pContext->mConfigPara.dstWidth, pContext->mConfigPara.dstHeight);

        ptr = (char *)GetConfParaString(&mConf, CFG_SRC_PIXFMT, NULL);
        if (ptr != NULL)
        {
            if (!strcmp(ptr, "nv21"))
            {
                pContext->mConfigPara.srcPixFmt = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
            }
            else if (!strcmp(ptr, "yv12"))
            {
                pContext->mConfigPara.srcPixFmt = MM_PIXEL_FORMAT_YVU_PLANAR_420;
            }
            else if (!strcmp(ptr, "nv12"))
            {
                pContext->mConfigPara.srcPixFmt = MM_PIXEL_FORMAT_YUV_SEMIPLANAR_420;
            }
            else if (!strcmp(ptr, "yu12"))
            {
                pContext->mConfigPara.srcPixFmt = MM_PIXEL_FORMAT_YUV_PLANAR_420;
            }
            else if (!strcmp(ptr, "aw_lbc_2_5x"))
            {
                pContext->mConfigPara.srcPixFmt = MM_PIXEL_FORMAT_YUV_AW_LBC_2_5X;
            }
            else if (!strcmp(ptr, "aw_lbc_2_0x"))
            {
                pContext->mConfigPara.srcPixFmt = MM_PIXEL_FORMAT_YUV_AW_LBC_2_0X;
            }
            else if (!strcmp(ptr, "aw_lbc_1_5x"))
            {
                pContext->mConfigPara.srcPixFmt = MM_PIXEL_FORMAT_YUV_AW_LBC_1_5X;
            }
            else if (!strcmp(ptr, "aw_lbc_1_0x"))
            {
                pContext->mConfigPara.srcPixFmt = MM_PIXEL_FORMAT_YUV_AW_LBC_1_0X;
            }
            else
            {
                aloge("fatal error! wrong src pixfmt:%s", ptr);
                alogw("use the default pixfmt %d", pContext->mConfigPara.srcPixFmt);
            }
        }

        ptr = (char *)GetConfParaString(&mConf, CFG_COLOR_SPACE, NULL);
        if (ptr != NULL)
        {
            if (!strcmp(ptr, "jpeg"))
            {
                pContext->mConfigPara.mColorSpace = V4L2_COLORSPACE_JPEG;
            }
            else if (!strcmp(ptr, "rec709"))
            {
                pContext->mConfigPara.mColorSpace = V4L2_COLORSPACE_REC709;
            }
            else if (!strcmp(ptr, "rec709_part_range"))
            {
                pContext->mConfigPara.mColorSpace = V4L2_COLORSPACE_REC709_PART_RANGE;
            }
            else
            {
                aloge("fatal error! wrong color space:%s", ptr);
                pContext->mConfigPara.mColorSpace = V4L2_COLORSPACE_JPEG;
            }
        }

        alogd("srcPixFmt=%d, ColorSpace=%d", pContext->mConfigPara.srcPixFmt, pContext->mConfigPara.mColorSpace);

        pContext->mConfigPara.mSaturationChange = GetConfParaInt(&mConf, CFG_SATURATION_CHANGE, 0);
        alogd("SaturationChange=%d", pContext->mConfigPara.mSaturationChange);
        
        ptr = (char *)GetConfParaString(&mConf, CFG_DST_VIDEO_FILE_STR, NULL);
        if (ptr != NULL)
        {
            strcpy(pContext->mConfigPara.dstVideoFile, ptr);
        }

        pContext->mConfigPara.mVideoFrameRate = GetConfParaInt(&mConf, CFG_DST_VIDEO_FRAMERATE, 0);
        pContext->mConfigPara.mViBufferNum = GetConfParaInt(&mConf, CFG_DST_VI_BUFFER_NUM, 0);
        pContext->mConfigPara.mVideoBitRate = GetConfParaInt(&mConf, CFG_DST_VIDEO_BITRATE, 0);

        pContext->mConfigPara.mProductMode = GetConfParaInt(&mConf, CFG_PRODUCT_MODE, 0);
        pContext->mConfigPara.mSensorType = GetConfParaInt(&mConf, CFG_SENSOR_TYPE, 0);
        pContext->mConfigPara.mKeyFrameInterval = GetConfParaInt(&mConf, CFG_KEY_FRAME_INTERVAL, 0);
        pContext->mConfigPara.mRcMode = GetConfParaInt(&mConf, CFG_RC_MODE, 0);
        pContext->mConfigPara.mInitQp = GetConfParaInt(&mConf, CFG_INIT_QP, 0);
        pContext->mConfigPara.mMinIQp = GetConfParaInt(&mConf, CFG_MIN_I_QP, 0);
        pContext->mConfigPara.mMaxIQp = GetConfParaInt(&mConf, CFG_MAX_I_QP, 0);
        pContext->mConfigPara.mMinPQp = GetConfParaInt(&mConf, CFG_MIN_P_QP, 0);
        pContext->mConfigPara.mMaxPQp = GetConfParaInt(&mConf, CFG_MAX_P_QP, 0);
        pContext->mConfigPara.mEnMbQpLimit = GetConfParaInt(&mConf, CFG_MB_QP_LIMIT, 0);
        pContext->mConfigPara.mMovingTh = GetConfParaInt(&mConf, CFG_MOVING_TH, 0);
        pContext->mConfigPara.mQuality = GetConfParaInt(&mConf, CFG_QUALITY, 0);
        pContext->mConfigPara.mPBitsCoef = GetConfParaInt(&mConf, CFG_P_BITS_COEF, 0);
        pContext->mConfigPara.mIBitsCoef = GetConfParaInt(&mConf, CFG_I_BITS_COEF, 0);
        pContext->mConfigPara.mGopMode = GetConfParaInt(&mConf, CFG_GOP_MODE, 0);
        pContext->mConfigPara.mGopSize = GetConfParaInt(&mConf, CFG_GOP_SIZE, 0);
        pContext->mConfigPara.mAdvancedRef_Base = GetConfParaInt(&mConf, CFG_AdvancedRef_Base, 0);
        pContext->mConfigPara.mAdvancedRef_Enhance = GetConfParaInt(&mConf, CFG_AdvancedRef_Enhance, 0);
        pContext->mConfigPara.mAdvancedRef_RefBaseEn = GetConfParaInt(&mConf, CFG_AdvancedRef_RefBaseEn, 0);
        pContext->mConfigPara.mEnableFastEnc = GetConfParaInt(&mConf, CFG_FAST_ENC, 0);
        pContext->mConfigPara.mbEnableSmart = GetConfParaBoolean(&mConf, CFG_ENABLE_SMART, 0);
        pContext->mConfigPara.mSVCLayer = GetConfParaInt(&mConf, CFG_SVC_LAYER, 0);
        pContext->mConfigPara.mEncodeRotate = GetConfParaInt(&mConf, CFG_ENCODE_ROTATE, 0);

        pContext->mConfigPara.m2DnrPara.enable_2d_filter = GetConfParaInt(&mConf, CFG_2DNR_EN, 0);
        pContext->mConfigPara.m2DnrPara.filter_strength_y = GetConfParaInt(&mConf, CFG_2DNR_STRENGTH_Y, 0);
        pContext->mConfigPara.m2DnrPara.filter_strength_uv = GetConfParaInt(&mConf, CFG_2DNR_STRENGTH_C, 0);
        pContext->mConfigPara.m2DnrPara.filter_th_y = GetConfParaInt(&mConf, CFG_2DNR_THRESHOLD_Y, 0);
        pContext->mConfigPara.m2DnrPara.filter_th_uv = GetConfParaInt(&mConf, CFG_2DNR_THRESHOLD_C, 0);

        pContext->mConfigPara.m3DnrPara.enable_3d_filter = GetConfParaInt(&mConf, CFG_3DNR_EN, 0);
        pContext->mConfigPara.m3DnrPara.adjust_pix_level_enable = GetConfParaInt(&mConf, CFG_3DNR_PIX_LEVEL_EN, 0);
        pContext->mConfigPara.m3DnrPara.smooth_filter_enable = GetConfParaInt(&mConf, CFG_3DNR_SMOOTH_EN, 0);
        pContext->mConfigPara.m3DnrPara.max_pix_diff_th = GetConfParaInt(&mConf, CFG_3DNR_PIX_DIFF_TH, 0);
        pContext->mConfigPara.m3DnrPara.max_mv_th = GetConfParaInt(&mConf, CFG_3DNR_MAX_MV_TH, 0);
        pContext->mConfigPara.m3DnrPara.max_mad_th = GetConfParaInt(&mConf, CFG_3DNR_MAX_MAD_TH, 0);
        pContext->mConfigPara.m3DnrPara.min_coef = GetConfParaInt(&mConf, CFG_3DNR_MIN_COEF, 0);
        pContext->mConfigPara.m3DnrPara.max_coef = GetConfParaInt(&mConf, CFG_3DNR_MAX_COEF, 0);

        ptr = (char *)GetConfParaString(&mConf, CFG_DST_VIDEO_ENCODER, NULL);
        if (ptr != NULL)
        {
            if (!strcmp(ptr, "H.264"))
            {
                pContext->mConfigPara.mVideoEncoderFmt = PT_H264;
                alogd("H.264");
            }
            else if (!strcmp(ptr, "H.265"))
            {
                pContext->mConfigPara.mVideoEncoderFmt = PT_H265;
                alogd("H.265");
            }
            else if (!strcmp(ptr, "MJPEG"))
            {
                pContext->mConfigPara.mVideoEncoderFmt = PT_MJPEG;
                alogd("MJPEG");
            }
            else
            {
                aloge("error conf encoder type");
            }
        }

        pContext->mConfigPara.mNormalTestDuration = GetConfParaInt(&mConf, CFG_NORMAL_TEST_DURATION, 0);
        pContext->mConfigPara.mEncUseProfile = GetConfParaInt(&mConf, CFG_DST_ENCODE_PROFILE, 0);
        alogd("vipp:%d, frame rate:%d, bitrate:%d, test_time=%d, profile=%d", pContext->mConfigPara.mVippDev,\
            pContext->mConfigPara.mVideoFrameRate, pContext->mConfigPara.mVideoBitRate,\
            pContext->mConfigPara.mNormalTestDuration, pContext->mConfigPara.mEncUseProfile);

        pContext->mConfigPara.mHorizonFlipFlag = GetConfParaInt(&mConf, CFG_MIRROR, 0);

        ptr = (char *)GetConfParaString(&mConf, CFG_COLOR2GREY, NULL);
        if (ptr != NULL)
        {
            if(!strcmp(ptr, "yes"))
            {
                pContext->mConfigPara.mColor2Grey = TRUE;
            }
            else
            {
                pContext->mConfigPara.mColor2Grey = FALSE;
            }
        }

        pContext->mConfigPara.mRoiNum = GetConfParaInt(&mConf, CFG_ROI_NUM, 0);
        pContext->mConfigPara.mRoiQp = GetConfParaInt(&mConf, CFG_ROI_QP, 0);
        pContext->mConfigPara.mRoiBgFrameRateEnable = GetConfParaBoolean(&mConf, CFG_ROI_BgFrameRateEnable, 0);
        pContext->mConfigPara.mRoiBgFrameRateAttenuation = GetConfParaInt(&mConf, CFG_ROI_BgFrameRateAttenuation, 0);
        pContext->mConfigPara.mIntraRefreshBlockNum = GetConfParaInt(&mConf, CFG_IntraRefresh_BlockNum, 0);
        pContext->mConfigPara.mOrlNum = GetConfParaInt(&mConf, CFG_ORL_NUM, 0);
        pContext->mConfigPara.mVbvBufferSize = GetConfParaInt(&mConf, CFG_vbvBufferSize, 0);
        pContext->mConfigPara.mVbvThreshSize = GetConfParaInt(&mConf, CFG_vbvThreshSize, 0);

        alogd("mirror:%d, Color2Grey:%d, RoiNum:%d, RoiQp:%d, RoiBgFrameRate Enable:%d Attenuation:%d, IntraRefreshBlockNum:%d, OrlNum:%d"
            "VbvBufferSize:%d, VbvThreshSize:%d",
            pContext->mConfigPara.mHorizonFlipFlag, pContext->mConfigPara.mColor2Grey,
            pContext->mConfigPara.mRoiNum, pContext->mConfigPara.mRoiQp,
            pContext->mConfigPara.mRoiBgFrameRateEnable, pContext->mConfigPara.mRoiBgFrameRateAttenuation,
            pContext->mConfigPara.mIntraRefreshBlockNum,
            pContext->mConfigPara.mOrlNum, pContext->mConfigPara.mVbvBufferSize,
            pContext->mConfigPara.mVbvThreshSize);

        pContext->mConfigPara.mCropEnable = GetConfParaInt(&mConf, CFG_CROP_ENABLE, 0);
        pContext->mConfigPara.mCropRectX = GetConfParaInt(&mConf, CFG_CROP_RECT_X, 0);
        pContext->mConfigPara.mCropRectY = GetConfParaInt(&mConf, CFG_CROP_RECT_Y, 0);
        pContext->mConfigPara.mCropRectWidth = GetConfParaInt(&mConf, CFG_CROP_RECT_WIDTH, 0);
        pContext->mConfigPara.mCropRectHeight = GetConfParaInt(&mConf, CFG_CROP_RECT_HEIGHT, 0);

        alogd("venc crop enable:%d, X:%d, Y:%d, Width:%d, Height:%d",
            pContext->mConfigPara.mCropEnable, pContext->mConfigPara.mCropRectX,
            pContext->mConfigPara.mCropRectY, pContext->mConfigPara.mCropRectWidth,
            pContext->mConfigPara.mCropRectHeight);

        pContext->mConfigPara.mVuiTimingInfoPresentFlag = GetConfParaInt(&mConf, CFG_vui_timing_info_present_flag, 0);
        alogd("VuiTimingInfoPresentFlag:%d", pContext->mConfigPara.mVuiTimingInfoPresentFlag);

        pContext->mConfigPara.mVeFreq = GetConfParaInt(&mConf, CFG_Ve_Freq, 0);
        alogd("mVeFreq:%d MHz", pContext->mConfigPara.mVeFreq);

        pContext->mConfigPara.mOnlineEnable = GetConfParaInt(&mConf, CFG_online_en, 0);
        pContext->mConfigPara.mOnlineShareBufNum = GetConfParaInt(&mConf, CFG_online_share_buf_num, 0);
        alogd("OnlineEnable: %d, OnlineShareBufNum: %d", pContext->mConfigPara.mOnlineEnable,
            pContext->mConfigPara.mOnlineShareBufNum);

        if (0 == pContext->mConfigPara.mOnlineEnable)
        {
            // venc drop frame only support offline.
            pContext->mConfigPara.mViDropFrameNum = GetConfParaInt(&mConf, CFG_DROP_FRAME_NUM, 0);
            alogd("ViDropFrameNum: %d", pContext->mConfigPara.mViDropFrameNum);
        }
        else
        {
            // venc drop frame support online and offline.
            pContext->mConfigPara.mVencDropFrameNum = GetConfParaInt(&mConf, CFG_DROP_FRAME_NUM, 0);
            alogd("VencDropFrameNum: %d", pContext->mConfigPara.mVencDropFrameNum);
        }

        pContext->mConfigPara.wdr_en = GetConfParaInt(&mConf, CFG_WDR_EN, 0);
        alogd("wdr_en: %d", pContext->mConfigPara.wdr_en);

        pContext->mConfigPara.mEnableGdc = GetConfParaInt(&mConf, CFG_EnableGdc, 0);
        alogd("EnableGdc: %d", pContext->mConfigPara.mEnableGdc);

        // Venc Recreate
        pContext->mDynamicConfig.mVencRecreateEnable = GetConfParaInt(&mConf, CFG_EnableVencRecreate, 0);
        ptr = (char *)GetConfParaString(&mConf, CFG_DynamicVideoEncoder, NULL);
        if (ptr != NULL)
        {
            if (!strcmp(ptr, "H.264"))
            {
                pContext->mDynamicConfig.mVideoEncoderFmt = PT_H264;
                alogd("H.264");
            }
            else if (!strcmp(ptr, "H.265"))
            {
                pContext->mDynamicConfig.mVideoEncoderFmt = PT_H265;
                alogd("H.265");
            }
            else
            {
                aloge("error conf encoder type");
            }
        }
        pContext->mDynamicConfig.mVideoFrameRate = GetConfParaInt(&mConf, CFG_DynamicVideoFramerate, 0);
        pContext->mDynamicConfig.mVideoBitRate = GetConfParaInt(&mConf, CFG_DynamicVideoBitrate, 0);
        pContext->mDynamicConfig.dstWidth = GetConfParaInt(&mConf, CFG_DynamicVideoWidth, 0);
        pContext->mDynamicConfig.dstHeight = GetConfParaInt(&mConf, CFG_DynamicVideoHeight, 0);
        pContext->mDynamicConfig.mKeyFrameInterval = GetConfParaInt(&mConf, CFG_DynamicKeyFrameInterval, 0);
        int dynamicRcMode = GetConfParaInt(&mConf, CFG_DynamicRcMode, 0);
        if (PT_H264 == pContext->mDynamicConfig.mVideoEncoderFmt)
        {
            if (0 == dynamicRcMode)
                pContext->mDynamicConfig.mRcMode = VENC_RC_MODE_H264CBR;
            else if (1 == dynamicRcMode)
                pContext->mDynamicConfig.mRcMode = VENC_RC_MODE_H264VBR;
            else
                aloge("error conf rc mode");
        }
        else if (PT_H265 == pContext->mDynamicConfig.mVideoEncoderFmt)
        {
            if (0 == dynamicRcMode)
                pContext->mDynamicConfig.mRcMode = VENC_RC_MODE_H265CBR;
            else if (1 == dynamicRcMode)
                pContext->mDynamicConfig.mRcMode = VENC_RC_MODE_H265VBR;
            else
                aloge("error conf rc mode");
        }
        pContext->mDynamicConfig.mVencRecreateTestDuration = GetConfParaInt(&mConf, CFG_VencRecreateTestDuration, 0);
        alogd("EnableVencRecreate: %d, VencRecreateTestDuration: %d",
            pContext->mDynamicConfig.mVencRecreateEnable, pContext->mDynamicConfig.mVencRecreateTestDuration);
        alogd("EncoderFmt: %d, FrameRate: %d, BitRate: %d, Width: %d, Height: %d, KeyFrameInterval: %d, RcMode: %d",
            pContext->mDynamicConfig.mVideoEncoderFmt,
            pContext->mDynamicConfig.mVideoFrameRate,
            pContext->mDynamicConfig.mVideoBitRate,
            pContext->mDynamicConfig.dstWidth,
            pContext->mDynamicConfig.dstHeight,
            pContext->mDynamicConfig.mKeyFrameInterval,
            pContext->mDynamicConfig.mRcMode);

        destroyConfParser(&mConf);
    }

    return SUCCESS;
}

static ERRORTYPE MPPCallbackWrapper(void *cookie, MPP_CHN_S *pChn, MPP_EVENT_TYPE event, void *pEventData)
{
    SAMPLE_VENC_RECREATE_S *pContext = (SAMPLE_VENC_RECREATE_S *)cookie;
    ERRORTYPE ret = 0;

    if (MOD_ID_VENC == pChn->mModId)
    {
        VENC_CHN mVEncChn = pChn->mChnId;
        switch(event)
        {
            case MPP_EVENT_LINKAGE_ISP2VE_PARAM:
            {
                VencIsp2VeParam *pIsp2VeParam = (VencIsp2VeParam *)pEventData;
                if (pIsp2VeParam)
                {
                    sEncppSharpParam *pSharpParam = &pIsp2VeParam->mSharpParam;
                    ISP_DEV mIspDev = 0;
                    ret = AW_MPI_VI_GetIspDev(pContext->mConfigPara.mVippDev, &mIspDev);
                    if (ret)
                    {
                        aloge("fatal error, vipp[%d] GetIspDev failed! ret=%d", pContext->mConfigPara.mVippDev, ret);
                        return -1;
                    }
                    struct enc_VencIsp2VeParam mIsp2VeParam;
                    memset(&mIsp2VeParam, 0, sizeof(struct enc_VencIsp2VeParam));
                    ret = AW_MPI_ISP_GetIsp2VeParam(mIspDev, &mIsp2VeParam);
                    if (ret)
                    {
                        aloge("fatal error, isp[%d] GetIsp2VeParam failed! ret=%d", mIspDev, ret);
                        return -1;
                    }

                    if (mIsp2VeParam.encpp_en)
                    {
                        VENC_CHN_ATTR_S stVencAttr;
                        memset(&stVencAttr, 0, sizeof(VENC_CHN_ATTR_S));
                        AW_MPI_VENC_GetChnAttr(mVEncChn, &stVencAttr);
                        if (FALSE == stVencAttr.EncppAttr.mbEncppEnable)
                        {
                            stVencAttr.EncppAttr.mbEncppEnable = TRUE;
                            AW_MPI_VENC_SetChnAttr(mVEncChn, &stVencAttr);
                        }
                        memcpy(&pSharpParam->mDynamicParam, &mIsp2VeParam.mDynamicSharpCfg,sizeof(sEncppSharpParamDynamic));
                        memcpy(&pSharpParam->mStaticParam, &mIsp2VeParam.mStaticSharpCfg, sizeof(sEncppSharpParamStatic));
                    }
                    else
                    {
                        VENC_CHN_ATTR_S stVencAttr;
                        memset(&stVencAttr, 0, sizeof(VENC_CHN_ATTR_S));
                        AW_MPI_VENC_GetChnAttr(mVEncChn, &stVencAttr);
                        if (TRUE == stVencAttr.EncppAttr.mbEncppEnable)
                        {
                            stVencAttr.EncppAttr.mbEncppEnable = FALSE;
                            AW_MPI_VENC_SetChnAttr(mVEncChn, &stVencAttr);
                        }
                    }

                    pIsp2VeParam->mEnvLv = AW_MPI_ISP_GetEnvLV(mIspDev);
                    pIsp2VeParam->mAeWeightLum = AW_MPI_ISP_GetAeWeightLum(mIspDev);
                    pIsp2VeParam->mEnCameraMove = CAMERA_ADAPTIVE_STATIC;
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }

    return SUCCESS;
}

static inline unsigned int map_H264_UserSet2Profile(int val)
{
    unsigned int profile = (unsigned int)H264_PROFILE_HIGH;
    switch (val)
    {
    case 0:
        profile = (unsigned int)H264_PROFILE_BASE;
        break;

    case 1:
        profile = (unsigned int)H264_PROFILE_MAIN;
        break;

    case 2:
        profile = (unsigned int)H264_PROFILE_HIGH;
        break;

    default:
        break;
    }

    return profile;
}

static inline unsigned int map_H265_UserSet2Profile(int val)
{
    unsigned int profile = H265_PROFILE_MAIN;
    switch (val)
    {
    case 0:
        profile = (unsigned int)H265_PROFILE_MAIN;
        break;

    case 1:
        profile = (unsigned int)H265_PROFILE_MAIN10;
        break;

    case 2:
        profile = (unsigned int)H265_PROFILE_STI11;
        break;

    default:
        break;
    }
    return profile;
}

static void initGdcParam(sGdcParam *pGdcParam)
{
    pGdcParam->bGDC_en = 1;
    pGdcParam->eWarpMode = Gdc_Warp_LDC;
    pGdcParam->eMountMode = Gdc_Mount_Wall;
    pGdcParam->bMirror = 0;

    pGdcParam->fx = 2417.19;
    pGdcParam->fy = 2408.43;
    pGdcParam->cx = 1631.50;
    pGdcParam->cy = 1223.50;
    pGdcParam->fx_scale = 2161.82;
    pGdcParam->fy_scale = 2153.99;
    pGdcParam->cx_scale = 1631.50;
    pGdcParam->cy_scale = 1223.50;

    pGdcParam->eLensDistModel = Gdc_DistModel_FishEye;

    pGdcParam->distCoef_wide_ra[0] = -0.3849;
    pGdcParam->distCoef_wide_ra[1] = 0.1567;
    pGdcParam->distCoef_wide_ra[2] = -0.0030;
    pGdcParam->distCoef_wide_ta[0] = -0.00005;
    pGdcParam->distCoef_wide_ta[1] = 0.0016;

    pGdcParam->distCoef_fish_k[0]  = -0.0024;
    pGdcParam->distCoef_fish_k[1]  = 0.141;
    pGdcParam->distCoef_fish_k[2]  = -0.3;
    pGdcParam->distCoef_fish_k[3]  = 0.2328;

    pGdcParam->centerOffsetX         =      0;     //[-255,0]
    pGdcParam->centerOffsetY         =      0;     //[-255,0]
    pGdcParam->rotateAngle           =      0;     //[0,360]
    pGdcParam->radialDistortCoef     =      0;     //[-255,255]
    pGdcParam->trapezoidDistortCoef  =      0;     //[-255,255]
    pGdcParam->fanDistortCoef        =      0;     //[-255,255]
    pGdcParam->pan                   =      0;     //pano360:[0,360]; others:[-90,90]
    pGdcParam->tilt                  =      0;     //[-90,90]
    pGdcParam->zoomH                 =      100;   //[0,100]
    pGdcParam->zoomV                 =      100;   //[0,100]
    pGdcParam->scale                 =      100;   //[0,100]
    pGdcParam->innerRadius           =      0;     //[0,width/2]
    pGdcParam->roll                  =      0;     //[-90,90]
    pGdcParam->pitch                 =      0;     //[-90,90]
    pGdcParam->yaw                   =      0;     //[-90,90]

    pGdcParam->perspFunc             =    Gdc_Persp_Only;
    pGdcParam->perspectiveProjMat[0] =    1.0;
    pGdcParam->perspectiveProjMat[1] =    0.0;
    pGdcParam->perspectiveProjMat[2] =    0.0;
    pGdcParam->perspectiveProjMat[3] =    0.0;
    pGdcParam->perspectiveProjMat[4] =    1.0;
    pGdcParam->perspectiveProjMat[5] =    0.0;
    pGdcParam->perspectiveProjMat[6] =    0.0;
    pGdcParam->perspectiveProjMat[7] =    0.0;
    pGdcParam->perspectiveProjMat[8] =    1.0;

    pGdcParam->mountHeight           =      0.85; //meters
    pGdcParam->roiDist_ahead         =      4.5;  //meters
    pGdcParam->roiDist_left          =     -1.5;  //meters
    pGdcParam->roiDist_right         =      1.5;  //meters
    pGdcParam->roiDist_bottom        =      0.65; //meters

    pGdcParam->peaking_en            =      1;    //0/1
    pGdcParam->peaking_clamp         =      1;    //0/1
    pGdcParam->peak_m                =     16;    //[0,63]
    pGdcParam->th_strong_edge        =      6;    //[0,15]
    pGdcParam->peak_weights_strength =      2;    //[0,15]

    if (pGdcParam->eWarpMode == Gdc_Warp_LDC)
    {
        pGdcParam->birdsImg_width    = 768;
        pGdcParam->birdsImg_height   = 1080;
    }
}

static ERRORTYPE configVencChnAttr(SAMPLE_VENC_RECREATE_S *pContext)
{
    memset(&pContext->mVencChnAttr, 0, sizeof(VENC_CHN_ATTR_S));
    if (pContext->mConfigPara.mOnlineEnable)
    {
        pContext->mVencChnAttr.VeAttr.mOnlineEnable = 1;
        pContext->mVencChnAttr.VeAttr.mOnlineShareBufNum = pContext->mConfigPara.mOnlineShareBufNum;
    }
    pContext->mVencChnAttr.VeAttr.Type = pContext->mConfigPara.mVideoEncoderFmt;
    pContext->mVencChnAttr.VeAttr.MaxKeyInterval = pContext->mConfigPara.mKeyFrameInterval;
    pContext->mVencChnAttr.VeAttr.SrcPicWidth  = pContext->mConfigPara.srcWidth;
    pContext->mVencChnAttr.VeAttr.SrcPicHeight = pContext->mConfigPara.srcHeight;
    pContext->mVencChnAttr.VeAttr.Field = VIDEO_FIELD_FRAME;
    pContext->mVencChnAttr.VeAttr.PixelFormat = pContext->mConfigPara.srcPixFmt;
    pContext->mVencChnAttr.VeAttr.mColorSpace = pContext->mConfigPara.mColorSpace;
    alogd("pixfmt:0x%x, colorSpace:0x%x", pContext->mVencChnAttr.VeAttr.PixelFormat, pContext->mVencChnAttr.VeAttr.mColorSpace);
    pContext->mVencChnAttr.VeAttr.mDropFrameNum = pContext->mConfigPara.mVencDropFrameNum;
    alogd("DropFrameNum:%d", pContext->mVencChnAttr.VeAttr.mDropFrameNum);
    pContext->mVencChnAttr.EncppAttr.mbEncppEnable = TRUE;
    switch(pContext->mConfigPara.mEncodeRotate)
    {
        case 90:
            pContext->mVencChnAttr.VeAttr.Rotate = ROTATE_90;
            break;
        case 180:
            pContext->mVencChnAttr.VeAttr.Rotate = ROTATE_180;
            break;
        case 270:
            pContext->mVencChnAttr.VeAttr.Rotate = ROTATE_270;
            break;
        default:
            pContext->mVencChnAttr.VeAttr.Rotate = ROTATE_NONE;
            break;
    }

    pContext->mVencRcParam.product_mode = pContext->mConfigPara.mProductMode;
    pContext->mVencRcParam.sensor_type = pContext->mConfigPara.mSensorType;
    if (PT_H264 == pContext->mVencChnAttr.VeAttr.Type)
    {
        pContext->mVencChnAttr.VeAttr.AttrH264e.BufSize = pContext->mConfigPara.mVbvBufferSize;
        pContext->mVencChnAttr.VeAttr.AttrH264e.mThreshSize = pContext->mConfigPara.mVbvThreshSize;
        pContext->mVencChnAttr.VeAttr.AttrH264e.bByFrame = TRUE;
        pContext->mVencChnAttr.VeAttr.AttrH264e.Profile = map_H264_UserSet2Profile(pContext->mConfigPara.mEncUseProfile);
        pContext->mVencChnAttr.VeAttr.AttrH264e.mLevel = 0; /* set the default value 0 and encoder will adjust automatically. */
        pContext->mVencChnAttr.VeAttr.AttrH264e.PicWidth  = pContext->mConfigPara.dstWidth;
        pContext->mVencChnAttr.VeAttr.AttrH264e.PicHeight = pContext->mConfigPara.dstHeight;
        pContext->mVencChnAttr.VeAttr.AttrH264e.mbPIntraEnable = TRUE;
        switch (pContext->mConfigPara.mRcMode)
        {
        case 1:
            pContext->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264VBR;
            pContext->mVencRcParam.ParamH264Vbr.mMinQp = pContext->mConfigPara.mMinIQp;
            pContext->mVencRcParam.ParamH264Vbr.mMaxQp = pContext->mConfigPara.mMaxIQp;
            pContext->mVencChnAttr.RcAttr.mAttrH264Vbr.mMaxBitRate = pContext->mConfigPara.mVideoBitRate;
            pContext->mVencRcParam.ParamH264Vbr.mMaxPqp = pContext->mConfigPara.mMaxPQp;
            pContext->mVencRcParam.ParamH264Vbr.mMinPqp = pContext->mConfigPara.mMinPQp;
            pContext->mVencRcParam.ParamH264Vbr.mQpInit = pContext->mConfigPara.mInitQp;
            pContext->mVencRcParam.ParamH264Vbr.mbEnMbQpLimit = pContext->mConfigPara.mEnMbQpLimit;
            pContext->mVencRcParam.ParamH264Vbr.mMovingTh = pContext->mConfigPara.mMovingTh;
            pContext->mVencRcParam.ParamH264Vbr.mQuality = pContext->mConfigPara.mQuality;
            pContext->mVencRcParam.ParamH264Vbr.mIFrmBitsCoef = pContext->mConfigPara.mIBitsCoef;
            pContext->mVencRcParam.ParamH264Vbr.mPFrmBitsCoef = pContext->mConfigPara.mPBitsCoef;
            break;
        case 2:
            pContext->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264FIXQP;
            pContext->mVencChnAttr.RcAttr.mAttrH264FixQp.mIQp = pContext->mConfigPara.mMinIQp;
            pContext->mVencChnAttr.RcAttr.mAttrH264FixQp.mPQp = pContext->mConfigPara.mMinPQp;
            break;
        case 3:
            pContext->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264ABR;
            pContext->mVencChnAttr.RcAttr.mAttrH264Abr.mMaxBitRate = pContext->mConfigPara.mVideoBitRate;
            pContext->mVencChnAttr.RcAttr.mAttrH264Abr.mRatioChangeQp = 85;
            pContext->mVencChnAttr.RcAttr.mAttrH264Abr.mQuality = 8;
            pContext->mVencChnAttr.RcAttr.mAttrH264Abr.mMinIQp = 20;
            pContext->mVencChnAttr.RcAttr.mAttrH264Abr.mMinQp = pContext->mConfigPara.mMinIQp;
            pContext->mVencChnAttr.RcAttr.mAttrH264Abr.mMaxQp = pContext->mConfigPara.mMaxIQp;
            break;
        case 0:
        default:
            pContext->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264CBR;
            pContext->mVencChnAttr.RcAttr.mAttrH264Cbr.mBitRate = pContext->mConfigPara.mVideoBitRate;
            pContext->mVencRcParam.ParamH264Cbr.mMaxQp = pContext->mConfigPara.mMaxIQp;
            pContext->mVencRcParam.ParamH264Cbr.mMinQp = pContext->mConfigPara.mMinIQp;
            pContext->mVencRcParam.ParamH264Cbr.mMaxPqp = pContext->mConfigPara.mMaxPQp;
            pContext->mVencRcParam.ParamH264Cbr.mMinPqp = pContext->mConfigPara.mMinPQp;
            pContext->mVencRcParam.ParamH264Cbr.mQpInit = pContext->mConfigPara.mInitQp;
            pContext->mVencRcParam.ParamH264Cbr.mbEnMbQpLimit = pContext->mConfigPara.mEnMbQpLimit;
            break;
        }
        if (pContext->mConfigPara.mEnableFastEnc)
        {
            pContext->mVencChnAttr.VeAttr.AttrH264e.FastEncFlag = TRUE;
        }
    }
    else if (PT_H265 == pContext->mVencChnAttr.VeAttr.Type)
    {
        pContext->mVencChnAttr.VeAttr.AttrH265e.mBufSize = pContext->mConfigPara.mVbvBufferSize;
        pContext->mVencChnAttr.VeAttr.AttrH265e.mThreshSize = pContext->mConfigPara.mVbvThreshSize;
        pContext->mVencChnAttr.VeAttr.AttrH265e.mbByFrame = TRUE;
        pContext->mVencChnAttr.VeAttr.AttrH265e.mProfile = map_H265_UserSet2Profile(pContext->mConfigPara.mEncUseProfile);
        pContext->mVencChnAttr.VeAttr.AttrH265e.mLevel = 0; /* set the default value 0 and encoder will adjust automatically. */
        pContext->mVencChnAttr.VeAttr.AttrH265e.mPicWidth = pContext->mConfigPara.dstWidth;
        pContext->mVencChnAttr.VeAttr.AttrH265e.mPicHeight = pContext->mConfigPara.dstHeight;
        pContext->mVencChnAttr.VeAttr.AttrH265e.mbPIntraEnable = TRUE;
        switch (pContext->mConfigPara.mRcMode)
        {
        case 1:
            pContext->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H265VBR;
            pContext->mVencRcParam.ParamH265Vbr.mMinQp = pContext->mConfigPara.mMinIQp;
            pContext->mVencRcParam.ParamH265Vbr.mMaxQp = pContext->mConfigPara.mMaxIQp;
            pContext->mVencChnAttr.RcAttr.mAttrH265Vbr.mMaxBitRate = pContext->mConfigPara.mVideoBitRate;
            pContext->mVencRcParam.ParamH265Vbr.mMaxPqp = pContext->mConfigPara.mMaxPQp;
            pContext->mVencRcParam.ParamH265Vbr.mMinPqp = pContext->mConfigPara.mMinPQp;
            pContext->mVencRcParam.ParamH265Vbr.mQpInit = pContext->mConfigPara.mInitQp;
            pContext->mVencRcParam.ParamH265Vbr.mbEnMbQpLimit = pContext->mConfigPara.mEnMbQpLimit;
            pContext->mVencRcParam.ParamH265Vbr.mMovingTh = pContext->mConfigPara.mMovingTh;
            pContext->mVencRcParam.ParamH265Vbr.mQuality = pContext->mConfigPara.mQuality;
            pContext->mVencRcParam.ParamH265Vbr.mIFrmBitsCoef = pContext->mConfigPara.mIBitsCoef;
            pContext->mVencRcParam.ParamH265Vbr.mPFrmBitsCoef = pContext->mConfigPara.mPBitsCoef;
            break;
        case 2:
            pContext->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H265FIXQP;
            pContext->mVencChnAttr.RcAttr.mAttrH265FixQp.mIQp = pContext->mConfigPara.mMinIQp;
            pContext->mVencChnAttr.RcAttr.mAttrH265FixQp.mPQp = pContext->mConfigPara.mMinPQp;
            break;
        case 3:
            pContext->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H265ABR;
            pContext->mVencChnAttr.RcAttr.mAttrH265Abr.mMaxBitRate = pContext->mConfigPara.mVideoBitRate;
            pContext->mVencChnAttr.RcAttr.mAttrH265Abr.mRatioChangeQp = 85;
            pContext->mVencChnAttr.RcAttr.mAttrH265Abr.mQuality = pContext->mConfigPara.mQuality;
            pContext->mVencChnAttr.RcAttr.mAttrH265Abr.mMinIQp = pContext->mConfigPara.mMinIQp;
            pContext->mVencChnAttr.RcAttr.mAttrH265Abr.mMinQp = pContext->mConfigPara.mMinIQp;
            pContext->mVencChnAttr.RcAttr.mAttrH265Abr.mMaxQp = pContext->mConfigPara.mMaxIQp;
            break;
        case 0:
        default:
            pContext->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H265CBR;
            pContext->mVencChnAttr.RcAttr.mAttrH265Cbr.mBitRate = pContext->mConfigPara.mVideoBitRate;
            pContext->mVencRcParam.ParamH265Cbr.mMaxQp = pContext->mConfigPara.mMaxIQp;
            pContext->mVencRcParam.ParamH265Cbr.mMinQp = pContext->mConfigPara.mMinIQp;
            pContext->mVencRcParam.ParamH265Cbr.mMaxPqp = pContext->mConfigPara.mMaxPQp;
            pContext->mVencRcParam.ParamH265Cbr.mMinPqp = pContext->mConfigPara.mMinPQp;
            pContext->mVencRcParam.ParamH265Cbr.mQpInit = pContext->mConfigPara.mInitQp;
            pContext->mVencRcParam.ParamH265Cbr.mbEnMbQpLimit = pContext->mConfigPara.mEnMbQpLimit;
            break;
        }
        if (pContext->mConfigPara.mEnableFastEnc)
        {
            pContext->mVencChnAttr.VeAttr.AttrH265e.mFastEncFlag = TRUE;
        }
    }
    else if (PT_MJPEG == pContext->mVencChnAttr.VeAttr.Type)
    {
        pContext->mVencChnAttr.VeAttr.AttrMjpeg.mBufSize = pContext->mConfigPara.mVbvBufferSize;
        pContext->mVencChnAttr.VeAttr.AttrMjpeg.mbByFrame = TRUE;
        pContext->mVencChnAttr.VeAttr.AttrMjpeg.mPicWidth = pContext->mConfigPara.dstWidth;
        pContext->mVencChnAttr.VeAttr.AttrMjpeg.mPicHeight = pContext->mConfigPara.dstHeight;
        switch (pContext->mConfigPara.mRcMode)
        {
        case 0:
            pContext->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_MJPEGCBR;
            pContext->mVencChnAttr.RcAttr.mAttrMjpegeCbr.mBitRate = pContext->mConfigPara.mVideoBitRate;
            break;
        case 1:
            pContext->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_MJPEGFIXQP;
            pContext->mVencChnAttr.RcAttr.mAttrMjpegeFixQp.mQfactor = 40;
            break;
        case 2:
        case 3:
            aloge("not support! use default cbr mode");
            pContext->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_MJPEGCBR;
            break;
        default:
            pContext->mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_MJPEGCBR;
            break;
        }
        pContext->mVencChnAttr.RcAttr.mAttrMjpegeCbr.mBitRate = pContext->mConfigPara.mVideoBitRate;
    }

    alogd("venc set Rcmode=%d", pContext->mVencChnAttr.RcAttr.mRcMode);

    if(0 == pContext->mConfigPara.mGopMode)
    {
        pContext->mVencChnAttr.GopAttr.enGopMode = VENC_GOPMODE_NORMALP;
    }
    else if(1 == pContext->mConfigPara.mGopMode)
    {
        pContext->mVencChnAttr.GopAttr.enGopMode = VENC_GOPMODE_DUALP;
    }
    else if(2 == pContext->mConfigPara.mGopMode)
    {
        pContext->mVencChnAttr.GopAttr.enGopMode = VENC_GOPMODE_SMARTP;
        pContext->mVencChnAttr.GopAttr.stSmartP.mVirtualIFrameInterval = 15;
    }
    pContext->mVencChnAttr.GopAttr.mGopSize = pContext->mConfigPara.mGopSize;

    if (pContext->mConfigPara.mEnableGdc)
    {
        alogd("enable GDC and init GDC params");
        initGdcParam(&pContext->mVencChnAttr.GdcAttr);
    }

    return SUCCESS;
}

static ERRORTYPE createVencChn(SAMPLE_VENC_RECREATE_S *pContext)
{
    ERRORTYPE ret;
    BOOL nSuccessFlag = FALSE;

    configVencChnAttr(pContext);
    if (pContext->mConfigPara.mOnlineEnable)
    {
        pContext->mVeChn = 0;
        alogd("online: only vipp0 & Vechn0 support online.");
    }
    else
    {
        pContext->mVeChn = pContext->mConfigPara.mVeChn;
    }

    while (pContext->mVeChn < VENC_MAX_CHN_NUM)
    {
        ret = AW_MPI_VENC_CreateChn(pContext->mVeChn, &pContext->mVencChnAttr);
        if (SUCCESS == ret)
        {
            nSuccessFlag = TRUE;
            alogd("create venc channel[%d] success!", pContext->mVeChn);
            break;
        }
        else if (ERR_VENC_EXIST == ret)
        {
            alogd("venc channel[%d] is exist, find next!", pContext->mVeChn);
            pContext->mVeChn++;
        }
        else
        {
            alogd("create venc channel[%d] ret[0x%x], find next!", pContext->mVeChn, ret);
            pContext->mVeChn++;
        }
    }

    if (nSuccessFlag == FALSE)
    {
        pContext->mVeChn = MM_INVALID_CHN;
        aloge("fatal error! create venc channel fail!");
        return FAILURE;
    }
    else
    {
        if (0 < pContext->mConfigPara.mVeFreq)
        {
            AW_MPI_VENC_SetVEFreq(pContext->mVeChn, pContext->mConfigPara.mVeFreq);
            alogd("set VE freq %d MHz", pContext->mConfigPara.mVeFreq);
        }

        AW_MPI_VENC_SetRcParam(pContext->mVeChn, &pContext->mVencRcParam);

        VENC_FRAME_RATE_S stFrameRate;
        stFrameRate.SrcFrmRate = stFrameRate.DstFrmRate = pContext->mConfigPara.mVideoFrameRate;
        alogd("set venc framerate: src %dfps, dst %dfps", stFrameRate.SrcFrmRate, stFrameRate.DstFrmRate);
        memcpy(&pContext->mstFrameRate, &stFrameRate, sizeof(VENC_FRAME_RATE_S));
        AW_MPI_VENC_SetFrameRate(pContext->mVeChn, &stFrameRate);

        if (pContext->mConfigPara.mAdvancedRef_Base)
        {
            VENC_PARAM_REF_S stRefParam;
            memset(&stRefParam, 0, sizeof(VENC_PARAM_REF_S));
            stRefParam.Base = pContext->mConfigPara.mAdvancedRef_Base;
            stRefParam.Enhance = pContext->mConfigPara.mAdvancedRef_Enhance;
            stRefParam.bEnablePred = pContext->mConfigPara.mAdvancedRef_RefBaseEn;
            AW_MPI_VENC_SetRefParam(pContext->mVeChn, &stRefParam);
            alogd("set RefParam %d %d %d", stRefParam.Base, stRefParam.Enhance, stRefParam.bEnablePred);
        }

        if (0 == pContext->mConfigPara.m2DnrPara.enable_2d_filter)
        {
            AW_MPI_VENC_Set2DFilter(pContext->mVeChn, &pContext->mConfigPara.m2DnrPara);
            alogd("disable and set 2DFilter param");
        }

        if (0 == pContext->mConfigPara.m3DnrPara.enable_3d_filter)
        {
            AW_MPI_VENC_Set3DFilter(pContext->mVeChn, &pContext->mConfigPara.m3DnrPara);
            alogd("disable and set 3DFilter param");
        }

        if (pContext->mConfigPara.mColor2Grey)
        {
            VENC_COLOR2GREY_S bColor2Grey;
            memset(&bColor2Grey, 0, sizeof(VENC_COLOR2GREY_S));
            bColor2Grey.bColor2Grey = pContext->mConfigPara.mColor2Grey;
            AW_MPI_VENC_SetColor2Grey(pContext->mVeChn, &bColor2Grey);
            alogd("set Color2Grey %d", pContext->mConfigPara.mColor2Grey);
        }

        if (pContext->mConfigPara.mHorizonFlipFlag)
        {
            AW_MPI_VENC_SetHorizonFlip(pContext->mVeChn, pContext->mConfigPara.mHorizonFlipFlag);
            alogd("set HorizonFlip %d", pContext->mConfigPara.mHorizonFlipFlag);
        }

        if (pContext->mConfigPara.mCropEnable)
        {
            VENC_CROP_CFG_S stCropCfg;
            memset(&stCropCfg, 0, sizeof(VENC_CROP_CFG_S));
            stCropCfg.bEnable = pContext->mConfigPara.mCropEnable;
            stCropCfg.Rect.X = pContext->mConfigPara.mCropRectX;
            stCropCfg.Rect.Y = pContext->mConfigPara.mCropRectY;
            stCropCfg.Rect.Width = pContext->mConfigPara.mCropRectWidth;
            stCropCfg.Rect.Height = pContext->mConfigPara.mCropRectHeight;
            AW_MPI_VENC_SetCrop(pContext->mVeChn, &stCropCfg);
            alogd("set Crop %d, [%d][%d][%d][%d]", stCropCfg.bEnable, stCropCfg.Rect.X, stCropCfg.Rect.Y, stCropCfg.Rect.Width, stCropCfg.Rect.Height);
        }

        //test PIntraRefresh
        if(pContext->mConfigPara.mIntraRefreshBlockNum > 0)
        {
            VENC_PARAM_INTRA_REFRESH_S stIntraRefresh;
            memset(&stIntraRefresh, 0, sizeof(VENC_PARAM_INTRA_REFRESH_S));
            stIntraRefresh.bRefreshEnable = TRUE;
            stIntraRefresh.RefreshLineNum = pContext->mConfigPara.mIntraRefreshBlockNum;
            ret = AW_MPI_VENC_SetIntraRefresh(pContext->mVeChn, &stIntraRefresh);
            if(ret != SUCCESS)
            {
                aloge("fatal error! set roiBgFrameRate fail[0x%x]!", ret);
            }
            else
            {
                alogd("set intra refresh:%d", stIntraRefresh.RefreshLineNum);
            }
        }

        if(pContext->mConfigPara.mbEnableSmart)
        {
            VencSmartFun smartParam;
            memset(&smartParam, 0, sizeof(VencSmartFun));
            smartParam.smart_fun_en = 1;
            smartParam.img_bin_en = 1;
            smartParam.img_bin_th = 0;
            smartParam.shift_bits = 2;
            AW_MPI_VENC_SetSmartP(pContext->mVeChn, &smartParam);
        }

        if(pContext->mConfigPara.mSVCLayer > 0)
        {
            VencH264SVCSkip stSVCSkip;
            memset(&stSVCSkip, 0, sizeof(VencH264SVCSkip));
            stSVCSkip.nTemporalSVC = pContext->mConfigPara.mSVCLayer;
            AW_MPI_VENC_SetH264SVCSkip(pContext->mVeChn, &stSVCSkip);
        }

        if (pContext->mConfigPara.mVuiTimingInfoPresentFlag)
        {
            /** must be call it before AW_MPI_VENC_GetH264SpsPpsInfo(unbind) and AW_MPI_VENC_StartRecvPic. */
            if(PT_H264 == pContext->mVencChnAttr.VeAttr.Type)
            {
                VENC_PARAM_H264_VUI_S H264Vui;
                memset(&H264Vui, 0, sizeof(VENC_PARAM_H264_VUI_S));
                AW_MPI_VENC_GetH264Vui(pContext->mVeChn, &H264Vui);
                H264Vui.VuiTimeInfo.timing_info_present_flag = 1;
                H264Vui.VuiTimeInfo.fixed_frame_rate_flag = 1;
                H264Vui.VuiTimeInfo.num_units_in_tick = 1000;
                H264Vui.VuiTimeInfo.time_scale = H264Vui.VuiTimeInfo.num_units_in_tick * pContext->mConfigPara.mVideoFrameRate * 2;
                AW_MPI_VENC_SetH264Vui(pContext->mVeChn, &H264Vui);
            }
            else if(PT_H265 == pContext->mVencChnAttr.VeAttr.Type)
            {
                VENC_PARAM_H265_VUI_S H265Vui;
                memset(&H265Vui, 0, sizeof(VENC_PARAM_H265_VUI_S));
                AW_MPI_VENC_GetH265Vui(pContext->mVeChn, &H265Vui);
                H265Vui.VuiTimeInfo.timing_info_present_flag = 1;
                H265Vui.VuiTimeInfo.num_units_in_tick = 1000;
                /* Notices: the protocol syntax states that h265 does not need to be multiplied by 2. */
                H265Vui.VuiTimeInfo.time_scale = H265Vui.VuiTimeInfo.num_units_in_tick * pContext->mConfigPara.mVideoFrameRate;
                H265Vui.VuiTimeInfo.num_ticks_poc_diff_one_minus1 = H265Vui.VuiTimeInfo.num_units_in_tick;
                AW_MPI_VENC_SetH265Vui(pContext->mVeChn, &H265Vui);
            }
        }

        MPPCallbackInfo cbInfo;
        cbInfo.cookie = (void*)pContext;
        cbInfo.callback = (MPPCallbackFuncType)&MPPCallbackWrapper;
        AW_MPI_VENC_RegisterCallback(pContext->mVeChn, &cbInfo);

        return SUCCESS;
    }
}

static ERRORTYPE createViChn(SAMPLE_VENC_RECREATE_S *pContext)
{
    ERRORTYPE ret;

    //create vi channel
    if (pContext->mConfigPara.mOnlineEnable)
    {
        pContext->mViDev = 0;
        alogd("online: only vipp0 & Vechn0 support online.");
    }
    else
    {
        pContext->mViDev = pContext->mConfigPara.mVippDev;
    }
    pContext->mIspDev = 0;
    pContext->mViChn = 0;

    ret = AW_MPI_VI_CreateVipp(pContext->mViDev);
    if (ret != SUCCESS)
    {
        aloge("fatal error! AW_MPI_VI CreateVipp failed");
    }

    memset(&pContext->mViAttr, 0, sizeof(VI_ATTR_S));
    if (pContext->mConfigPara.mOnlineEnable)
    {
        pContext->mViAttr.mOnlineEnable = 1;
        pContext->mViAttr.mOnlineShareBufNum = pContext->mConfigPara.mOnlineShareBufNum;
    }
    pContext->mViAttr.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    pContext->mViAttr.memtype = V4L2_MEMORY_MMAP;
    pContext->mViAttr.format.pixelformat = map_PIXEL_FORMAT_E_to_V4L2_PIX_FMT(pContext->mConfigPara.srcPixFmt);
    pContext->mViAttr.format.field = V4L2_FIELD_NONE;
    pContext->mViAttr.format.colorspace = pContext->mConfigPara.mColorSpace;
    pContext->mViAttr.format.width = pContext->mConfigPara.srcWidth;
    pContext->mViAttr.format.height = pContext->mConfigPara.srcHeight;
    pContext->mViAttr.nbufs =  pContext->mConfigPara.mViBufferNum;
    alogd("vipp use %d v4l2 buffers, colorspace: 0x%x", pContext->mViAttr.nbufs, pContext->mViAttr.format.colorspace);
    pContext->mViAttr.nplanes = 2;
    pContext->mViAttr.wdr_mode = pContext->mConfigPara.wdr_en;
    alogd("wdr_mode %d", pContext->mViAttr.wdr_mode);
    pContext->mViAttr.fps = pContext->mConfigPara.mVideoFrameRate;
    pContext->mViAttr.drop_frame_num = pContext->mConfigPara.mViDropFrameNum;
    pContext->mViAttr.mbEncppEnable = TRUE;

    ret = AW_MPI_VI_SetVippAttr(pContext->mViDev, &pContext->mViAttr);
    if (ret != SUCCESS)
    {
        aloge("fatal error! AW_MPI_VI SetVippAttr failed");
    }
#if ISP_RUN
    AW_MPI_ISP_Run(pContext->mIspDev);
#endif

    // Saturation change
    if (pContext->mConfigPara.mSaturationChange)
    {
        int nSaturationValue = 0;
        AW_MPI_ISP_GetSaturation(pContext->mIspDev, &nSaturationValue);
        alogd("current SaturationValue: %d", nSaturationValue);
        nSaturationValue = nSaturationValue + pContext->mConfigPara.mSaturationChange;
        AW_MPI_ISP_SetSaturation(pContext->mIspDev, nSaturationValue);
        AW_MPI_ISP_GetSaturation(pContext->mIspDev, &nSaturationValue);
        alogd("after change, SaturationValue: %d", nSaturationValue);
    }

    ViVirChnAttrS stVirChnAttr;
    memset(&stVirChnAttr, 0, sizeof(ViVirChnAttrS));
    stVirChnAttr.mbRecvInIdleState = TRUE;
    ret = AW_MPI_VI_CreateVirChn(pContext->mViDev, pContext->mViChn, &stVirChnAttr);
    if (ret != SUCCESS)
    {
        aloge("fatal error! createVirChn[%d] fail!", pContext->mViChn);
    }
    ret = AW_MPI_VI_EnableVipp(pContext->mViDev);
    if (ret != SUCCESS)
    {
        aloge("fatal error! enableVipp fail!");
    }
    return ret;
}

static ERRORTYPE prepare(SAMPLE_VENC_RECREATE_S *pContext)
{
    ERRORTYPE result = FAILURE;

    if (createViChn(pContext) != SUCCESS)
    {
        aloge("create vi chn fail");
        return result;
    }

    if (createVencChn(pContext) != SUCCESS)
    {
        aloge("create venc chn fail");
        return result;
    }

    pContext->mOutputFileFp = fopen(pContext->mConfigPara.dstVideoFile, "wb+");
    if (NULL == pContext->mOutputFileFp)
    {
        aloge("fatal error! can't open file[%s]", pContext->mConfigPara.dstVideoFile);
        return result;
    }
    else
    {
        alogd("open %s success", pContext->mConfigPara.dstVideoFile);
    }

    if ((pContext->mViDev >= 0 && pContext->mViChn >= 0) && pContext->mVeChn >= 0)
    {
        MPP_CHN_S ViChn = {MOD_ID_VIU, pContext->mViDev, pContext->mViChn};
        MPP_CHN_S VeChn = {MOD_ID_VENC, 0, pContext->mVeChn};

        AW_MPI_SYS_Bind(&ViChn, &VeChn);
    }

    return 0;
}

static ERRORTYPE start(SAMPLE_VENC_RECREATE_S *pContext)
{
    ERRORTYPE ret = SUCCESS;

    alogd("start");

    ret = AW_MPI_VI_EnableVirChn(pContext->mViDev, pContext->mViChn);
    if (ret != SUCCESS)
    {
        alogd("VI enable error!");
        return FAILURE;
    }

    if (pContext->mVeChn >= 0)
    {
        AW_MPI_VENC_StartRecvPic(pContext->mVeChn);
    }

    return ret;
}

static ERRORTYPE stop(SAMPLE_VENC_RECREATE_S *pContext)
{
    ERRORTYPE ret = SUCCESS;

    alogd("stop");

    if (pContext->mViChn >= 0)
    {
        AW_MPI_VI_DisableVirChn(pContext->mViDev, pContext->mViChn);
    }

    if (pContext->mVeChn >= 0)
    {
        alogd("stop venc");
        AW_MPI_VENC_StopRecvPic(pContext->mVeChn);
    }

    if (pContext->mVeChn >= 0)
    {
        alogd("destory venc");
        //AW_MPI_VENC_ResetChn(pContext->mVeChn);
        AW_MPI_VENC_DestroyChn(pContext->mVeChn);
        pContext->mVeChn = MM_INVALID_CHN;
    }
    if (pContext->mViChn >= 0)
    {
        AW_MPI_VI_DestroyVirChn(pContext->mViDev, pContext->mViChn);
        AW_MPI_VI_DisableVipp(pContext->mViDev);
    #if ISP_RUN
        AW_MPI_ISP_Stop(pContext->mIspDev);
    #endif
        AW_MPI_VI_DestroyVipp(pContext->mViDev);
    }

    if (pContext->mOutputFileFp)
    {
        fclose(pContext->mOutputFileFp);
        pContext->mOutputFileFp = NULL;
    }

    return SUCCESS;
}

static int vencRecreate(SAMPLE_VENC_RECREATE_S *pContext)
{
    ERRORTYPE ret = 0;
    if (NULL == pContext)
    {
        aloge("fatal error, pContext is NULL!");
        return -1;
    }

    alogd("vencRecreate process veChn[%d]!", pContext->mVeChn);
    if(pContext->mIsVencRecreateDone)
    {
        alogd("veChn[%d] has already recreate done!", pContext->mVeChn);
        return 0;
    }

    //(1)check if need recreate vencLib!
    // old config
    unsigned int nOldBitRate = GetBitRateFromVENC_CHN_ATTR_S(&pContext->mVencChnAttr);
    int nOldDstFrameRate = pContext->mstFrameRate.DstFrmRate;
    SIZE_S stOldDstEncodeSize = {0};
    ret = GetEncodeDstSizeFromVENC_CHN_ATTR_S(&pContext->mVencChnAttr, &stOldDstEncodeSize);
    if(ret != SUCCESS)
    {
        aloge("fatal error! get encode dst size fail! check code!");
    }
    int nOldKeyInterval = pContext->mVencChnAttr.VeAttr.MaxKeyInterval;
    VENC_RC_MODE_E nOldRcMode = pContext->mVencChnAttr.RcAttr.mRcMode;
    PAYLOAD_TYPE_E nOldEncodeType = pContext->mVencChnAttr.VeAttr.Type;

    // new config
    VENC_RECREATE_DYNAMIC_CONFIG_S *pNewDynamicConfig = &pContext->mDynamicConfig;
    unsigned int nNewBitRate = pNewDynamicConfig->mVideoBitRate;
    int nNewDstFrameRate = pNewDynamicConfig->mVideoFrameRate;
    SIZE_S stNewDstEncodeSize = {pNewDynamicConfig->dstWidth, pNewDynamicConfig->dstHeight};
    int nNewKeyInterval = pNewDynamicConfig->mKeyFrameInterval;
    VENC_RC_MODE_E nNewRcMode = pNewDynamicConfig->mRcMode;
    PAYLOAD_TYPE_E nNewEncodeType = pNewDynamicConfig->mVideoEncoderFmt;

    BOOL bNeedRecreate = FALSE;
    BOOL bNeedResetVipp = FALSE;
    do
    {
        if(nOldBitRate != nNewBitRate)
        {
            alogd("veChn[%d] bitRate change:[%d]->[%d]", pContext->mVeChn, nOldBitRate, nNewBitRate);
            bNeedRecreate = TRUE;
        }
        if(nOldDstFrameRate != nNewDstFrameRate)
        {
            alogd("veChn[%d] dstFrameRate change:[%d]->[%d]", pContext->mVeChn, nOldDstFrameRate, nNewDstFrameRate);
            bNeedRecreate = TRUE;
        }
        if(stOldDstEncodeSize.Width != stNewDstEncodeSize.Width || stOldDstEncodeSize.Height != stNewDstEncodeSize.Height)
        {
            alogd("veChn[%d] dstEncodeSize change:[%dx%d]->[%dx%d], vippSize[%dx%d]", pContext->mVeChn,
                stOldDstEncodeSize.Width, stOldDstEncodeSize.Height,
                stNewDstEncodeSize.Width, stNewDstEncodeSize.Height,
                pContext->mViAttr.format.width, pContext->mViAttr.format.height);
            bNeedRecreate = TRUE;
            bNeedResetVipp = TRUE;
        }
        if(nOldKeyInterval != nNewKeyInterval)
        {
            alogd("veChn[%d] keyFrameInterval change:[%d]->[%d]", pContext->mVeChn, nOldKeyInterval, nNewKeyInterval);
            bNeedRecreate = TRUE;
        }
        if(nOldRcMode != nNewRcMode)
        {
            alogd("veChn[%d] rcMode change:[%d]->[%d]", pContext->mVeChn, nOldRcMode, nNewRcMode);
            bNeedRecreate = TRUE;
        }
        if(nOldEncodeType!= nNewEncodeType)
        {
            alogd("veChn[%d] EncodeType change:[%d]->[%d]", pContext->mVeChn, nOldEncodeType, nNewEncodeType);
            bNeedRecreate = TRUE;
        }
    }while(0);

    //(2)recreate vencLib
    if (bNeedResetVipp)
    {
        //a) destroy vipp
        ret = AW_MPI_VI_DisableVirChn(pContext->mViDev, pContext->mViChn);
        if (ret != SUCCESS)
        {
            aloge("fatal error! vipp[%d] disable virChn[%d] fail[0x%x]!", pContext->mViDev, pContext->mViChn, ret);
        }
        ret = AW_MPI_VENC_StopRecvPic(pContext->mVeChn);
        if (ret != SUCCESS)
        {
            aloge("fatal error! vencChn[%d] stop fail[0x%x]!", pContext->mVeChn, ret);
        }
        //must return all inputFrames before unbind.
        ret = AW_MPI_VENC_ResetChn(pContext->mVeChn);
        if (ret != SUCCESS)
        {
            aloge("fatal error! vencChn[%d] ret fail[0x%x]!", pContext->mVeChn, ret);
        }
        //unbind virViChn and vencChn
        MPP_CHN_S stVirViChn = {MOD_ID_VIU, pContext->mViDev, pContext->mViChn};
        MPP_CHN_S stVencChn = {MOD_ID_VENC, 0, pContext->mVeChn};
        ret = AW_MPI_SYS_UnBind(&stVirViChn, &stVencChn);
        if (ret != SUCCESS)
        {
            aloge("fatal error! virViChn[%d-%d] && vencChn[%d] unbind fail[0x%x]!", pContext->mViDev, pContext->mViChn, pContext->mVeChn, ret);
        }
        ret = AW_MPI_VI_DestroyVirChn(pContext->mViDev, pContext->mViChn);
        if (ret != SUCCESS)
        {
            aloge("fatal error! vipp[%d] destroy virViChn[%d] fail[0x%x]!", pContext->mViDev, pContext->mViChn, ret);
        }
        ret = AW_MPI_VI_DisableVipp(pContext->mViDev);
        if (ret != SUCCESS)
        {
            aloge("fatal error! disableVipp[%d] fail!", pContext->mViDev);
        }
#if ISP_RUN
        AW_MPI_ISP_Stop(pContext->mIspDev);
#endif
        ret = AW_MPI_VI_DestroyVipp(pContext->mViDev);
        if (ret != SUCCESS)
        {
            aloge("fatal error! AW_MPI_VI DestroyVipp[%d] failed", pContext->mViDev);
        }

        //b) create vipp
        //update viAttr's resolution
        pContext->mViAttr.format.width = stNewDstEncodeSize.Width;
        pContext->mViAttr.format.height = stNewDstEncodeSize.Height;
        ret = AW_MPI_VI_CreateVipp(pContext->mViDev);
        if (ret != SUCCESS)
        {
            aloge("fatal error! AW_MPI_VI CreateVipp[%d] failed", pContext->mViDev);
        }
        MPPCallbackInfo cbInfo;
        cbInfo.cookie = (void*)pContext;
        cbInfo.callback = (MPPCallbackFuncType)&MPPCallbackWrapper;
        ret = AW_MPI_VI_RegisterCallback(pContext->mViDev, &cbInfo);
        if (ret != SUCCESS)
        {
            aloge("fatal error! AW_MPI_VI RegisterCallback[%d] failed", pContext->mViDev);
        }
        ret = AW_MPI_VI_SetVippAttr(pContext->mViDev, &pContext->mViAttr);
        if (ret != SUCCESS)
        {
            aloge("fatal error! AW_MPI_VI SetVippAttr[%d] failed", pContext->mViDev);
        }
        ret = AW_MPI_VI_GetVippAttr(pContext->mViDev, &pContext->mViAttr);
        if (ret != SUCCESS)
        {
            aloge("fatal error! AW_MPI_VI GetVippAttr[%d] failed", pContext->mViDev);
        }
#if ISP_RUN
        AW_MPI_ISP_Run(pContext->mIspDev);
#endif
        ret = AW_MPI_VI_EnableVipp(pContext->mViDev);
        if (ret != SUCCESS)
        {
            aloge("fatal error! enableVipp[%d] fail!", pContext->mViDev);
        }
        ret = AW_MPI_VI_CreateVirChn(pContext->mViDev, pContext->mViChn, NULL);
        if (ret != SUCCESS)
        {
            aloge("fatal error! vipp[%d] createVirChn[%d] fail!", pContext->mViDev, pContext->mViChn);
        }
        //bind vi to venc.
        ret = AW_MPI_SYS_Bind(&stVirViChn, &stVencChn);
        if (ret != SUCCESS)
        {
            aloge("fatal error! bind viChn[%d-%d] to vencChn[%d] fail!", pContext->mViDev, pContext->mViChn, pContext->mVeChn);
        }
        //start virViChn.
        ret = AW_MPI_VI_EnableVirChn(pContext->mViDev, pContext->mViChn);
        if (ret != SUCCESS)
        {
            aloge("fatal error! virViChn[%d-%d] enable error!", pContext->mViDev, pContext->mViChn);
        }
    }

    if (bNeedRecreate)
    {
        AW_MPI_VENC_StopRecvPic(pContext->mVeChn);
        //must return all outFrames before destroy encLib.
        AW_MPI_VENC_DestroyEncoder(pContext->mVeChn);
        //config VencConfig again.
        VENC_CHN_ATTR_S *pVencChnAttr = &pContext->mVencChnAttr;
        VENC_RC_PARAM_S *pVencRcParam = &pContext->mVencRcParam;
        unsigned int nThreshSize = stNewDstEncodeSize.Width*stNewDstEncodeSize.Height*3/2/5;
        unsigned int nBufSize = nNewBitRate/8*2 + nThreshSize;
        pVencChnAttr->VeAttr.Type = nNewEncodeType;
        pVencChnAttr->VeAttr.MaxKeyInterval = nNewKeyInterval;
        pVencChnAttr->VeAttr.SrcPicWidth = pContext->mViAttr.format.width;
        pVencChnAttr->VeAttr.SrcPicHeight = pContext->mViAttr.format.height;
        if (PT_H264 == pVencChnAttr->VeAttr.Type)
        {
            pVencChnAttr->VeAttr.AttrH264e.BufSize = ALIGN(nBufSize, 1024);
            pVencChnAttr->VeAttr.AttrH264e.mThreshSize = nThreshSize;
            pVencChnAttr->VeAttr.AttrH264e.bByFrame = TRUE;
            pVencChnAttr->VeAttr.AttrH264e.Profile = 2; //1//0:base 1:main 2:high
            pVencChnAttr->VeAttr.AttrH264e.mLevel = H264_LEVEL_51;
            pVencChnAttr->VeAttr.AttrH264e.PicWidth  = stNewDstEncodeSize.Width;
            pVencChnAttr->VeAttr.AttrH264e.PicHeight = stNewDstEncodeSize.Height;
            pVencChnAttr->VeAttr.AttrH264e.mbPIntraEnable = TRUE;
            pVencChnAttr->RcAttr.mRcMode = nNewRcMode;
            switch (pVencChnAttr->RcAttr.mRcMode)
            {
                case VENC_RC_MODE_H264VBR:
                {
                    pVencRcParam->ParamH264Vbr.mMinQp = 10;
                    pVencRcParam->ParamH264Vbr.mMaxQp = 45;
                    pVencChnAttr->RcAttr.mAttrH264Vbr.mMaxBitRate = nNewBitRate;
                    pVencRcParam->ParamH264Vbr.mMaxPqp = 50;
                    pVencRcParam->ParamH264Vbr.mMinPqp = 10;
                    pVencRcParam->ParamH264Vbr.mQpInit = 37;
                    pVencRcParam->ParamH264Vbr.mbEnMbQpLimit = 0;
                    pVencRcParam->ParamH264Vbr.mMovingTh = 20;
                    pVencRcParam->ParamH264Vbr.mQuality = 17;
                    pVencRcParam->ParamH264Vbr.mIFrmBitsCoef = 10;
                    pVencRcParam->ParamH264Vbr.mPFrmBitsCoef = 10;
                    break;
                }
                case VENC_RC_MODE_H264CBR:
                {
                    pVencChnAttr->RcAttr.mAttrH264Cbr.mBitRate = nNewBitRate;
                    pVencRcParam->ParamH264Cbr.mMaxQp = 35;
                    pVencRcParam->ParamH264Cbr.mMinQp = 20;
                    pVencRcParam->ParamH264Cbr.mMaxPqp = 45;
                    pVencRcParam->ParamH264Cbr.mMinPqp = 22;
                    pVencRcParam->ParamH264Cbr.mQpInit = 30;
                    pVencRcParam->ParamH264Cbr.mbEnMbQpLimit = 0;
                    break;
                }
                default:
                {
                    aloge("fatal error! wrong rcMode[%d], check code!", pVencChnAttr->RcAttr.mRcMode);
                    break;
                }
            }
        }
        else if (PT_H265 == pVencChnAttr->VeAttr.Type)
        {
            pVencChnAttr->VeAttr.AttrH265e.mBufSize = ALIGN(nBufSize, 1024);
            pVencChnAttr->VeAttr.AttrH265e.mThreshSize = nThreshSize;
            pVencChnAttr->VeAttr.AttrH265e.mbByFrame = TRUE;
            pVencChnAttr->VeAttr.AttrH265e.mProfile = 0;//0:main 1:main10 2:sti11
            pVencChnAttr->VeAttr.AttrH265e.mLevel = H265_LEVEL_62;
            pVencChnAttr->VeAttr.AttrH265e.mPicWidth = stNewDstEncodeSize.Width;
            pVencChnAttr->VeAttr.AttrH265e.mPicHeight = stNewDstEncodeSize.Height;
            pVencChnAttr->VeAttr.AttrH265e.mbPIntraEnable = TRUE;
            pVencChnAttr->RcAttr.mRcMode = nNewRcMode;
            switch (pVencChnAttr->RcAttr.mRcMode)
            {
                case VENC_RC_MODE_H265VBR:
                {
                    pVencRcParam->ParamH265Vbr.mMinQp = 10;
                    pVencRcParam->ParamH265Vbr.mMaxQp = 45;
                    pVencChnAttr->RcAttr.mAttrH265Vbr.mMaxBitRate = nNewBitRate;
                    pVencRcParam->ParamH265Vbr.mMaxPqp = 50;
                    pVencRcParam->ParamH265Vbr.mMinPqp = 10;
                    pVencRcParam->ParamH265Vbr.mQpInit = 37;
                    pVencRcParam->ParamH265Vbr.mbEnMbQpLimit = 0;
                    pVencRcParam->ParamH265Vbr.mMovingTh = 20;
                    pVencRcParam->ParamH265Vbr.mQuality = 17;
                    pVencRcParam->ParamH265Vbr.mIFrmBitsCoef = 10;
                    pVencRcParam->ParamH265Vbr.mPFrmBitsCoef = 10;
                    break;
                }
                case VENC_RC_MODE_H265CBR:
                {
                    pVencChnAttr->RcAttr.mAttrH265Cbr.mBitRate = nNewBitRate;
                    pVencRcParam->ParamH265Cbr.mMaxQp = 35;
                    pVencRcParam->ParamH265Cbr.mMinQp = 20;
                    pVencRcParam->ParamH265Cbr.mMaxPqp = 45;
                    pVencRcParam->ParamH265Cbr.mMinPqp = 22;
                    pVencRcParam->ParamH265Cbr.mQpInit = 30;
                    pVencRcParam->ParamH265Cbr.mbEnMbQpLimit = 0;
                    break;
                }
                default:
                {
                    aloge("fatal error! wrong rcMode[%d], check code!", pVencChnAttr->RcAttr.mRcMode);
                    break;
                }
            }
        }
        else
        {
            aloge("fatal error! main stream wrong venc type:%d", pVencChnAttr->VeAttr.Type);
        }
        pContext->mstFrameRate.DstFrmRate = nNewDstFrameRate;

        AW_MPI_VENC_SetChnAttr(pContext->mVeChn, pVencChnAttr);
        AW_MPI_VENC_SetRcParam(pContext->mVeChn, pVencRcParam);
        AW_MPI_VENC_SetFrameRate(pContext->mVeChn, &pContext->mstFrameRate);
        ret = AW_MPI_VENC_StartRecvPic(pContext->mVeChn);
        if (ret != SUCCESS)
        {
            aloge("fatal error! venc[%d] start fail!", pContext->mVeChn);
        }
    }
    pContext->mIsVencRecreateDone = TRUE;
    alogd("venc_recreate process venc handle[%d] done!", pContext->mVeChn);
    return ret;
}

static void *GetEncoderStreamThread(void *pThreadData)
{
    SAMPLE_VENC_RECREATE_S *pContext = NULL;
    int ret = 0;
    int count = 0;
    VENC_CHN vencChn = 0;
    FILE *outputFileFp = NULL;
    VencHeaderData SpsPpsInfo;

    if (NULL == pThreadData)
    {
        aloge("fatal error, pThreadData is NULL!");
        return NULL;
    }
    pContext = (SAMPLE_VENC_RECREATE_S*)pThreadData;
    vencChn = pContext->mVeChn;
    outputFileFp = pContext->mOutputFileFp;

    //set spspps
    memset(&SpsPpsInfo, 0, sizeof(VencHeaderData));
    if (PT_H264 == pContext->mConfigPara.mVideoEncoderFmt)
    {
        ret = AW_MPI_VENC_GetH264SpsPpsInfo(vencChn, &SpsPpsInfo);
        if (SUCCESS == ret)
        {
            if(SpsPpsInfo.nLength)
            {
                fwrite(SpsPpsInfo.pBuffer, 1, SpsPpsInfo.nLength, outputFileFp);
            }
        }
        else
        {
            aloge("fatal error! get spspps fail[0x%x]!", ret);
        }
    }
    else if (PT_H265 == pContext->mConfigPara.mVideoEncoderFmt)
    {
        ret = AW_MPI_VENC_GetH265SpsPpsInfo(vencChn, &SpsPpsInfo);
        if (SUCCESS == ret)
        {
            if (SpsPpsInfo.nLength)
            {
                fwrite(SpsPpsInfo.pBuffer, 1, SpsPpsInfo.nLength, outputFileFp);
            }
        }
        else
        {
            aloge("fatal error! get spspps fail[0x%x]!", ret);
        }
    }

    VENC_STREAM_S vencFrame;
    VENC_PACK_S venc_pack;
    vencFrame.mPackCount = 1;
    vencFrame.mpPack = &venc_pack;

    pContext->mDynamicConfig.mVencRecreateEnable = 0;

    while (0 == pContext->mExitFlag)
    {
        if (pContext->mDynamicConfig.mVencRecreateEnable)
        {
            do
            {
                //(1) check if recreate done!
                if (pContext->mIsVencRecreateDone)
                {
                    break;
                }
                //(2) recreate this venc handle!
                ret = vencRecreate(pContext);
                if (SUCCESS != ret)
                {
                    aloge("fatal error! why venc recreate fail[%d]?", ret);
                }
                // update spspps.
                memset(&SpsPpsInfo, 0, sizeof(VencHeaderData));
                if (PT_H264 == pContext->mConfigPara.mVideoEncoderFmt)
                {
                    ret = AW_MPI_VENC_GetH264SpsPpsInfo(vencChn, &SpsPpsInfo);
                    if (ret != SUCCESS)
                    {
                        aloge("fatal error! get spspps fail[0x%x]!", ret);
                    }
                }
                else if(PT_H265 == pContext->mConfigPara.mVideoEncoderFmt)
                {
                    ret = AW_MPI_VENC_GetH265SpsPpsInfo(vencChn, &SpsPpsInfo);
                    if (ret != SUCCESS)
                    {
                        aloge("fatal error! get spspps fail[0x%x]!", ret);
                    }
                }
            }while(0);
        }
    
        count++;
        if ((ret = AW_MPI_VENC_GetStream(vencChn, &vencFrame, 4000)) < 0)
        {
            aloge("fatal error! get frmae fail[0x%x]!", ret);
            continue;
        }
        else
        {
            if (vencFrame.mpPack != NULL && vencFrame.mpPack->mLen0)
            {
                fwrite(vencFrame.mpPack->mpAddr0,1,vencFrame.mpPack->mLen0, outputFileFp);
            }
            if (vencFrame.mpPack != NULL && vencFrame.mpPack->mLen1)
            {
                fwrite(vencFrame.mpPack->mpAddr1,1,vencFrame.mpPack->mLen1, outputFileFp);
            }
            ret = AW_MPI_VENC_ReleaseStream(vencChn, &vencFrame);
            if (ret < 0)
            {
                aloge("fatal error! release frmae fail[0x%x]!", ret);
            }
         }
    }

    return NULL;
}

int main(int argc, char** argv)
{
    int result = -1;
    GLogConfig stGLogConfig = 
    {
        .FLAGS_logtostderr = 1,
        .FLAGS_colorlogtostderr = 1,
        .FLAGS_stderrthreshold = _GLOG_INFO,
        .FLAGS_minloglevel = _GLOG_INFO,
        .FLAGS_logbuflevel = -1,
        .FLAGS_logbufsecs = 0,
        .FLAGS_max_log_size = 1,
        .FLAGS_stop_logging_if_full_disk = 1,
    };
    strcpy(stGLogConfig.LogDir, "/tmp/log");
    strcpy(stGLogConfig.InfoLogFileNameBase, "LOG-");
    strcpy(stGLogConfig.LogFileNameExtension, "IPC-");
    log_init(argv[0], &stGLogConfig);

    printf("sample_virvi2venc running!\n");
    SAMPLE_VENC_RECREATE_S *pContext = (SAMPLE_VENC_RECREATE_S* )malloc(sizeof(SAMPLE_VENC_RECREATE_S));

    if (pContext == NULL)
    {
        aloge("malloc struct fail");
        result = FAILURE;
        goto _err0;
    }
    if (InitVirvi2VencData(pContext) != SUCCESS)
    {
        return -1;
    }
    gpVirvi2VencData = pContext;
    cdx_sem_init(&pContext->mSemExit, 0);

    /* register process function for SIGINT, to exit program. */
    if (signal(SIGINT, handle_exit) == SIG_ERR)
    {
        aloge("can't catch SIGSEGV");
    }

    if (parseCmdLine(pContext, argc, argv) != SUCCESS)
    {
        aloge("parse cmdline fail");
        result = FAILURE;
        goto err_out_0;
    }
    char *pConfPath = NULL;
    if(argc > 1)
    {
        pConfPath = pContext->mCmdLinePara.mConfigFilePath;
    }
        
    if (loadConfigPara(pContext, pConfPath) != SUCCESS)
    {
        aloge("load config file fail");
        result = FAILURE;
        goto err_out_0;
    }
    alogd("ViDropFrameNum=%d", pContext->mConfigPara.mViDropFrameNum);

    pContext->mSysConf.nAlignWidth = 32;
    AW_MPI_SYS_SetConf(&pContext->mSysConf);
    AW_MPI_SYS_Init();

    if (prepare(pContext) != SUCCESS)
    {
        aloge("prepare fail!");
        goto err_out_3;
    }

    result = pthread_create(&pContext->mThreadId, NULL, GetEncoderStreamThread, pContext);
    if (result != 0)
    {
        aloge("fatal error! create Msg Queue Thread fail[%d]", result);
        goto err_out_3;
    }
    else
    {
        alogd("create Msg Queue Thread success! threadId[0x%x]", &pContext->mThreadId);
    }

    start(pContext);

    //test roi.
    int i = 0;
    ERRORTYPE ret;
    VENC_ROI_CFG_S stMppRoiBlockInfo;
    memset(&stMppRoiBlockInfo, 0, sizeof(VENC_ROI_CFG_S));
    for(i=0; i<pContext->mConfigPara.mRoiNum; i++)
    {
        stMppRoiBlockInfo.Index = i;
        stMppRoiBlockInfo.bEnable = TRUE;
        stMppRoiBlockInfo.bAbsQp = TRUE;
        stMppRoiBlockInfo.Qp = pContext->mConfigPara.mRoiQp;
        stMppRoiBlockInfo.Rect.X = 128*i;
        stMppRoiBlockInfo.Rect.Y = 128*i;
        stMppRoiBlockInfo.Rect.Width = 128;
        stMppRoiBlockInfo.Rect.Height = 128;
        ret = AW_MPI_VENC_SetRoiCfg(pContext->mVeChn, &stMppRoiBlockInfo);
        if(ret != SUCCESS)
        {
            aloge("fatal error! set roi[%d] fail[0x%x]!", i, ret);
        }
        else
        {
            alogd("set roiIndex:%d, Qp:%d-%d, Rect[%d,%d,%dx%d]", i, stMppRoiBlockInfo.bAbsQp, stMppRoiBlockInfo.Qp, 
                stMppRoiBlockInfo.Rect.X, stMppRoiBlockInfo.Rect.Y, stMppRoiBlockInfo.Rect.Width, stMppRoiBlockInfo.Rect.Height);
        }
    }
    
    if(pContext->mConfigPara.mRoiNum>0 && pContext->mConfigPara.mRoiBgFrameRateEnable)
    {
        VENC_ROIBG_FRAME_RATE_S stRoiBgFrmRate;
        ret = AW_MPI_VENC_GetRoiBgFrameRate(pContext->mVeChn, &stRoiBgFrmRate);
        if(ret != SUCCESS)
        {
            aloge("fatal error! get roiBgFrameRate fail[0x%x]!", ret);
        }
        alogd("get roi bg frame rate:%d-%d", stRoiBgFrmRate.mSrcFrmRate, stRoiBgFrmRate.mDstFrmRate);
        if (pContext->mConfigPara.mRoiBgFrameRateAttenuation)
        {
            stRoiBgFrmRate.mDstFrmRate = stRoiBgFrmRate.mSrcFrmRate/pContext->mConfigPara.mRoiBgFrameRateAttenuation;
        }
        else
        {
            stRoiBgFrmRate.mDstFrmRate = stRoiBgFrmRate.mSrcFrmRate;
        }
        if(stRoiBgFrmRate.mDstFrmRate <= 0)
        {
            stRoiBgFrmRate.mDstFrmRate = 1;
        }
        ret = AW_MPI_VENC_SetRoiBgFrameRate(pContext->mVeChn, &stRoiBgFrmRate);
        if(ret != SUCCESS)
        {
            aloge("fatal error! set roiBgFrameRate fail[0x%x]!", ret);
        }
        alogd("set roi bg frame rate param:%d-%d", stRoiBgFrmRate.mSrcFrmRate, stRoiBgFrmRate.mDstFrmRate);
    }

    //test orl
    RGN_ATTR_S stRgnAttr;
    RGN_CHN_ATTR_S stRgnChnAttr;
    memset(&stRgnAttr, 0, sizeof(RGN_ATTR_S));
    memset(&stRgnChnAttr, 0, sizeof(RGN_CHN_ATTR_S));
    MPP_CHN_S viChn = {MOD_ID_VIU, pContext->mViDev, pContext->mViChn};
    for(i=0; i<pContext->mConfigPara.mOrlNum; i++)
    {
        stRgnAttr.enType = ORL_RGN;
        ret = AW_MPI_RGN_Create(i, &stRgnAttr);
        if(ret != SUCCESS)
        {
            aloge("fatal error! why create ORL region fail?[0x%x]", ret);
            break;
        }
        stRgnChnAttr.bShow = TRUE;
        stRgnChnAttr.enType = ORL_RGN;
        stRgnChnAttr.unChnAttr.stOrlChn.enAreaType = AREA_RECT;
        stRgnChnAttr.unChnAttr.stOrlChn.stRect.X = i*120;
        stRgnChnAttr.unChnAttr.stOrlChn.stRect.Y = i*60;
        stRgnChnAttr.unChnAttr.stOrlChn.stRect.Width = 100;
        stRgnChnAttr.unChnAttr.stOrlChn.stRect.Height = 50;
        stRgnChnAttr.unChnAttr.stOrlChn.mColor = 0xFF0000 >> ((i % 3)*8);
        stRgnChnAttr.unChnAttr.stOrlChn.mThick = 6;
        stRgnChnAttr.unChnAttr.stOrlChn.mLayer = i;
        ret = AW_MPI_RGN_AttachToChn(i, &viChn, &stRgnChnAttr);
        if(ret != SUCCESS)
        {
            aloge("fatal error! why attach to vi channel[%d,%d] fail?", pContext->mViDev, pContext->mViChn);
        }
        
    }

    if(pContext->mConfigPara.mOrlNum > 0)
    {
        alogd("sleep 10s ...");
        sleep(10);
    }
    for(i=0; i<pContext->mConfigPara.mOrlNum; i++)
    {
        ret = AW_MPI_RGN_Destroy(i);
        if(ret != SUCCESS)
        {
            aloge("fatal error! why destory region:%d fail?", i);
        }
    }

    if (pContext->mConfigPara.mNormalTestDuration > 0)
    {
        cdx_sem_down_timedwait(&pContext->mSemExit, pContext->mConfigPara.mNormalTestDuration*1000);
    }
    else
    {
        cdx_sem_down(&pContext->mSemExit);
    }
    pContext->mDynamicConfig.mVencRecreateEnable = 1;
    alogd("enable VencRecreate");
    if (pContext->mDynamicConfig.mVencRecreateTestDuration > 0)
    {
        cdx_sem_down_timedwait(&pContext->mSemExit, pContext->mDynamicConfig.mVencRecreateTestDuration*1000);
    }
    else
    {
        cdx_sem_down(&pContext->mSemExit);
    }

    pContext->mExitFlag = 1;    
    pthread_join(pContext->mThreadId, NULL);
    alogd("start to free res");
err_out_3:
    stop(pContext);
    result = 0;
err_out_1:
    AW_MPI_SYS_Exit();

err_out_0:
    cdx_sem_deinit(&pContext->mSemExit);
    free(pContext);
    gpVirvi2VencData = pContext = NULL;
_err0:
    alogd("%s test result: %s", argv[0], ((0 == result) ? "success" : "fail"));
    log_quit();
    return result;
}
