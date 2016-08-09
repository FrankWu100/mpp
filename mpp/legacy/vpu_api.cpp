/*
 * Copyright 2015 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define MODULE_TAG "vpu_api"

#include <string.h>
#ifdef RKPLATFORM
#include <dlfcn.h>
#include <unistd.h>
#endif

#include "mpp_log.h"
#include "mpp_mem.h"
#include "mpp_env.h"
#include "mpp_common.h"

#include "vpu_api.h"
#include "vpu_api_legacy.h"
#include "vpu_mem_legacy.h"

static RK_S32 vpu_api_init(VpuCodecContext *ctx, RK_U8 *extraData, RK_U32 extra_size)
{
    vpu_api_dbg_func("vpu_api_init in, extra_size: %d", extra_size);

    if (ctx == NULL) {
        mpp_log("vpu_api_init fail, input invalid");
        return VPU_API_ERR_UNKNOW;
    }
    VpuApiLegacy* api = (VpuApiLegacy*)(ctx->vpuApiObj);
    if (api == NULL) {
        mpp_log("vpu_api_init fail, vpu api invalid");
        return VPU_API_ERR_UNKNOW;
    }

    return api->init(ctx, extraData, extra_size);
}

static RK_S32 vpu_api_decode(VpuCodecContext *ctx, VideoPacket_t *pkt, DecoderOut_t *aDecOut)
{
    if (ctx == NULL) {
        mpp_log("vpu_api_decode fail, input invalid");
        return VPU_API_ERR_UNKNOW;
    }

    VpuApiLegacy* api = (VpuApiLegacy*)(ctx->vpuApiObj);
    if (api == NULL) {
        mpp_log("vpu_api_decode fail, vpu api invalid");
        return VPU_API_ERR_UNKNOW;
    }

    return api->decode(ctx, pkt, aDecOut);
}
static RK_S32 vpu_api_sendstream(VpuCodecContext *ctx, VideoPacket_t *pkt)
{
    if (ctx == NULL) {
        mpp_log("vpu_api_decode fail, input invalid");
        return VPU_API_ERR_UNKNOW;
    }

    VpuApiLegacy* api = (VpuApiLegacy*)(ctx->vpuApiObj);
    if (api == NULL) {
        mpp_log("vpu_api_sendstream fail, vpu api invalid");
        return VPU_API_ERR_UNKNOW;
    }

    return api->decode_sendstream(pkt);
}

static RK_S32 vpu_api_getframe(VpuCodecContext *ctx, DecoderOut_t *aDecOut)
{
    if (ctx == NULL) {
        mpp_log("vpu_api_decode fail, input invalid");
        return VPU_API_ERR_UNKNOW;
    }

    VpuApiLegacy* api = (VpuApiLegacy*)(ctx->vpuApiObj);
    if (api == NULL) {
        mpp_log("vpu_api_getframe fail, vpu api invalid");
        return VPU_API_ERR_UNKNOW;
    }

    return api->decode_getoutframe(aDecOut);
}

static RK_S32 vpu_api_sendframe(VpuCodecContext *ctx, EncInputStream_t *aEncInStrm)
{
    if (ctx == NULL) {
        mpp_log("vpu_api_decode fail, input invalid");
        return VPU_API_ERR_UNKNOW;
    }

    VpuApiLegacy* api = (VpuApiLegacy*)(ctx->vpuApiObj);
    if (api == NULL) {
        mpp_log("vpu_api_sendframe fail, vpu api invalid");
        return VPU_API_ERR_UNKNOW;
    }

    return api->encoder_sendframe(ctx, aEncInStrm);
}

static RK_S32 vpu_api_getstream(VpuCodecContext *ctx, EncoderOut_t *aEncOut)
{
    if (ctx == NULL) {
        mpp_log("vpu_api_decode fail, input invalid");
        return VPU_API_ERR_UNKNOW;
    }

    VpuApiLegacy* api = (VpuApiLegacy*)(ctx->vpuApiObj);
    if (api == NULL) {
        mpp_log("vpu_api_getframe fail, vpu api invalid");
        return VPU_API_ERR_UNKNOW;
    }

    return api->encoder_getstream(ctx, aEncOut);
}



static RK_S32 vpu_api_encode(VpuCodecContext *ctx, EncInputStream_t *aEncInStrm, EncoderOut_t *aEncOut)
{
    if (ctx == NULL) {
        mpp_log("vpu_api_encode fail, input invalid");
        return VPU_API_ERR_UNKNOW;
    }

    VpuApiLegacy* api = (VpuApiLegacy*)(ctx->vpuApiObj);
    if (api == NULL) {
        mpp_log("vpu_api_encode fail, vpu api invalid");
        return VPU_API_ERR_UNKNOW;
    }

    return api->encode(ctx, aEncInStrm, aEncOut);
}

static RK_S32 vpu_api_flush(VpuCodecContext *ctx)
{

    if (ctx == NULL) {
        mpp_log("vpu_api_encode fail, input invalid");
        return VPU_API_ERR_UNKNOW;
    }

    VpuApiLegacy* api = (VpuApiLegacy*)(ctx->vpuApiObj);
    if (api == NULL) {
        mpp_log("vpu_api_flush fail, vpu api invalid");
        return VPU_API_ERR_UNKNOW;
    }

    return api->flush(ctx);
}

static RK_S32 vpu_api_control(VpuCodecContext *ctx, VPU_API_CMD cmdType, void *param)
{
    if (ctx == NULL) {
        mpp_log("vpu_api_decode fail, input invalid");
        return VPU_API_ERR_UNKNOW;
    }

    VpuApiLegacy* api = (VpuApiLegacy*)(ctx->vpuApiObj);
    if (api == NULL) {
        mpp_log("vpu_api_decode fail, vpu api invalid");
        return VPU_API_ERR_UNKNOW;
    }

    vpu_api_dbg_func("enter\n");
    switch (cmdType) {
    case VPU_API_SET_VPUMEM_CONTEXT: {
        vpu_display_mem_pool_impl *p_mempool = (vpu_display_mem_pool_impl *)param;

        param = (void*)p_mempool->group;
        break;
    }
    default: {
        break;
    }
    }

    vpu_api_dbg_func("pass to mpi\n");
    return api->control(ctx, cmdType, param);
}

#ifndef ARM_LINUX
#ifdef RKPLATFORM
static const char *codec_paths[] = {
    "/system/lib/librk_vpuapi.so",
    "/system/lib/librk_on2.so",
    "/usr/lib/librk_codec.so",
};

class VpulibDlsym
{
public:
    void *rkapi_hdl;
    RK_S32 (*rkvpu_open_cxt)(VpuCodecContext **ctx);
    RK_S32 (*rkvpu_close_cxt)(VpuCodecContext **ctx);
    VpulibDlsym() :
        rkapi_hdl(NULL),
        rkvpu_open_cxt(NULL),
        rkvpu_close_cxt(NULL)
    {
        RK_U32 i;

        for (i = 0; i < MPP_ARRAY_ELEMS(codec_paths); i++) {
            rkapi_hdl = dlopen(codec_paths[i], RTLD_LAZY);
            if (rkapi_hdl)
                break;
        }

        if (rkapi_hdl) {
            rkvpu_open_cxt  = (RK_S32 (*)(VpuCodecContext **ctx))dlsym(rkapi_hdl, "vpu_open_context");
            rkvpu_close_cxt = (RK_S32 (*)(VpuCodecContext **ctx))dlsym(rkapi_hdl, "vpu_close_context");
            mpp_log("dlopen vpu lib success\n");
        } else {
            mpp_err("dlopen vpu lib failed\n");
        }
    }

    ~VpulibDlsym() {
        if (rkapi_hdl) {
            dlclose(rkapi_hdl);
            mpp_log("dlclose vpu lib");
        }
    }
};

static VpulibDlsym gVpulib;

RK_S32 open_orign_vpu(VpuCodecContext **ctx)
{
    if (gVpulib.rkvpu_open_cxt && ctx) {
        return (gVpulib.rkvpu_open_cxt)(ctx);
    }
    return MPP_NOK;
}

RK_S32 close_orign_vpu(VpuCodecContext **ctx)
{
    if (gVpulib.rkvpu_close_cxt && ctx) {
        return (gVpulib.rkvpu_close_cxt)(ctx);
    }
    return MPP_NOK;
}
#endif
#endif
/*
 * old libvpu path will input a NULL pointer in *ctx
 * new libvpu path will input non-NULL pointer in *ctx
 */
RK_S32 vpu_open_context(VpuCodecContext **ctx)
{
    VpuCodecContext *s = *ctx;
    RK_U32 value = 0;
    RK_U32 use_mpp_mode = 0;
    mpp_env_get_u32("chg_orig", &value, 0);
    mpp_env_get_u32("use_mpp_mode", &use_mpp_mode, 0);
#ifndef ARM_LINUX
#ifdef RKPLATFORM
    value = value || (!!access("/dev/rkvdec", F_OK));

    if (s != NULL)
        value = (value & (s->videoCoding != OMX_RK_VIDEO_CodingHEVC));

    if (value || !s) {
        if (s) {
            free(s);
            s = NULL;
        }
        open_orign_vpu(&s);
        s->extra_cfg.reserved[0] = 1;
        *ctx = s;
        return MPP_OK;
    }
    if (s != NULL) {
        MppCtxType type = (s->codecType == CODEC_DECODER) ? (MPP_CTX_DEC) :
                          (s->codecType == CODEC_ENCODER) ? (MPP_CTX_ENC) : (MPP_CTX_BUTT);
        MppCodingType coding = (MppCodingType)s->videoCoding;
        vpu_api_dbg_func("videoCoding %x, width %d, height %d \n", s->videoCoding, s->width, s->height);
        if (MPP_OK == mpp_check_support_format(type, coding)
            || (s->videoCoding == OMX_RK_VIDEO_CodingAVC &&
                s->codecType == CODEC_DECODER && ((s->width > 1920) || (s->height > 1088) || use_mpp_mode))) {
            free(s);
#endif
#endif
            s = NULL;
            s = mpp_malloc(VpuCodecContext, 1);
            if (!s) {
                mpp_err("Input context has not been properly allocated");
                return -1;
            }
            memset(s, 0, sizeof(VpuCodecContext));
            s->enableparsing = 1;

            VpuApiLegacy* api = new VpuApiLegacy();

            if (api == NULL) {
                mpp_err("Vpu api object has not been properly allocated");
                return -1;
            }

            s->vpuApiObj = (void*)api;
            s->init = vpu_api_init;
            s->decode = vpu_api_decode;
            s->encode = vpu_api_encode;
            s->flush = vpu_api_flush;
            s->control = vpu_api_control;
            s->decode_sendstream = vpu_api_sendstream;
            s->decode_getframe = vpu_api_getframe;
            s->encoder_sendframe = vpu_api_sendframe;
            s->encoder_getstream = vpu_api_getstream;

            *ctx = s;
            return 0;
#ifndef ARM_LINUX
#ifdef RKPLATFORM
            if (s != NULL) {
                free(s);
                s = NULL;
            }
            open_orign_vpu(&s);
            s->extra_cfg.reserved[0] = 1;
            *ctx = s;
            return MPP_OK;
#endif
        }
    }
#endif

    if (!s->vpuApiObj) {
        mpp_err("Input context has not been properly allocated and is not NULL either");
        return -1;
    }
    return 0;
}

RK_S32 vpu_close_context(VpuCodecContext **ctx)
{
    vpu_api_dbg_func("enter\n");
    VpuCodecContext *s = *ctx;
    RK_U32 value;
    mpp_env_get_u32("chg_orig", &value, 0);
#ifndef ARM_LINUX
#ifdef RKPLATFORM
    if (value || s->extra_cfg.reserved[0]) {
        close_orign_vpu(ctx);
        mpp_log("org vpu_close_context ok");
        return MPP_OK;
    }
#endif
#endif
    if (s) {
        s->flush(s);
        VpuApiLegacy* api = (VpuApiLegacy*)(s->vpuApiObj);
        if (s->vpuApiObj) {
            delete api;
            s->vpuApiObj = NULL;
        }
        //mpp_free(s->extradata);  // TODO
        if (s->extradata_size > 0) { // TODO
            s->extradata_size = 0;
            s->extradata = NULL;
        }
        mpp_free(s->private_data);
        mpp_free(s);
        *ctx = s = NULL;
        vpu_api_dbg_func("leave\n");
    }
    return 0;
}
