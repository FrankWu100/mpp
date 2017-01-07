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

#define  MODULE_TAG "mpp_hal"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "mpp_mem.h"
#include "mpp_log.h"
#include "mpp_time.h"
#include "mpp_common.h"

#include "mpp.h"
#include "mpp_hal.h"
#include "mpp_frame_impl.h"

#include "hal_h263d_api.h"
#include "hal_h264d_api.h"
#include "hal_h265d_api.h"
#include "hal_vp9d_api.h"
#include "hal_avsd_api.h"
#include "hal_m2vd_api.h"
#include "hal_mpg4d_api.h"
#include "hal_vp8d_api.h"
#include "hal_h264e_api.h"
#include "hal_jpegd_api.h"

// for test and demo
#include "hal_dummy_dec_api.h"
#include "hal_dummy_enc_api.h"

typedef struct {
	HalDeviceId hal_id;
	const char *compatible;
} platform_vpu_desc;

static const platform_vpu_desc mpp_vpu_platforms[] = {
	{.hal_id = HAL_VDPU1, .compatible = "rk3288"},
	{.hal_id = HAL_VDPU2, .compatible = "rk3399"},
	{.hal_id = HAL_VDPU2, .compatible = "rk3328"},
};

#define MPP_VPU_PLATFORM_COUNT \
		(sizeof(mpp_vpu_platforms)/sizeof(platform_vpu_desc))

/*
 * all hardware api static register here
 */
static const MppHalApi *hw_apis[] = {
    &hal_api_avsd,
    &hal_api_h263d,
    &hal_api_h264d,
    &hal_api_h265d,
    &hal_api_m2vd,
    &hal_api_mpg4d,
    &hal_api_vp8d,
    &hal_api_vp9d,
    &hal_api_jpegd,
    &hal_api_h264e,	
    &hal_api_dummy_dec,
    &hal_api_dummy_enc,
};

typedef struct MppHalImpl_t {
    MppCtxType      type;
    MppCodingType   coding;
    MppBufSlots     frame_slots;
    MppBufSlots     packet_slots;

    void            *ctx;
    const MppHalApi *api;

    HalTaskGroup    tasks;
    RK_S32          task_count;
} MppHalImpl;


MPP_RET mpp_hal_init(MppHal *ctx, MppHalCfg *cfg)
{
    if (NULL == ctx || NULL == cfg) {
        mpp_err_f("found NULL input ctx %p cfg %p\n", ctx, cfg);
        return MPP_ERR_NULL_PTR;
    }
    *ctx = NULL;

    MppHalImpl *p = mpp_calloc(MppHalImpl, 1);
    if (NULL == p) {
        mpp_err_f("malloc failed\n");
        return MPP_ERR_MALLOC;
    }

    RK_U32 i;
    for (i = 0; i < MPP_ARRAY_ELEMS(hw_apis); i++) {
        if (cfg->type   == hw_apis[i]->type &&
            cfg->coding == hw_apis[i]->coding) {
            mpp_assert(cfg->task_count > 0);
            p->type         = cfg->type;
            p->coding       = cfg->coding;
            p->frame_slots  = cfg->frame_slots;
            p->packet_slots = cfg->packet_slots;
            p->api          = hw_apis[i];
            p->task_count   = cfg->task_count;
            p->ctx          = mpp_calloc_size(void, p->api->ctx_size);
            p->api->init(p->ctx, cfg);

            MPP_RET ret = hal_task_group_init(&p->tasks, p->type, p->task_count);
            if (ret) {
                mpp_err_f("hal_task_group_init failed ret %d\n", ret);
                break;
            }

            cfg->tasks = p->tasks;
            *ctx = p;
            return MPP_OK;
        }
    }

    mpp_err_f("could not found coding type %d\n", cfg->coding);
    mpp_free(p->ctx);
    mpp_free(p);

    return MPP_NOK;
}

MPP_RET mpp_hal_deinit(MppHal ctx)
{
    if (NULL == ctx) {
        mpp_err_f("found NULL input\n");
        return MPP_ERR_NULL_PTR;
    }

    MppHalImpl *p = (MppHalImpl*)ctx;
    p->api->deinit(p->ctx);
    mpp_free(p->ctx);
    if (p->tasks)
        hal_task_group_deinit(p->tasks);
    mpp_free(p);
    return MPP_OK;
}

MPP_RET mpp_hal_reg_gen(MppHal ctx, HalTaskInfo *task)
{
    if (NULL == ctx || NULL == task) {
        mpp_err_f("found NULL input ctx %p task %p\n", ctx, task);
        return MPP_ERR_NULL_PTR;
    }

    MppHalImpl *p = (MppHalImpl*)ctx;
    MPP_RET ret = p->api->reg_gen(p->ctx, task);
    return ret;
}

MPP_RET mpp_hal_hw_start(MppHal ctx, HalTaskInfo *task)
{
    if (NULL == ctx || NULL == task) {
        mpp_err_f("found NULL input ctx %p task %p\n", ctx, task);
        return MPP_ERR_NULL_PTR;
    }

    MppHalImpl *p = (MppHalImpl*)ctx;
    MPP_RET ret = p->api->start(p->ctx, task);
    return ret;
}

MPP_RET mpp_hal_hw_wait(MppHal ctx, HalTaskInfo *task)
{
    if (NULL == ctx || NULL == task) {
        mpp_err_f("found NULL input ctx %p task %p\n", ctx, task);
        return MPP_ERR_NULL_PTR;
    }

    MppHalImpl *p = (MppHalImpl*)ctx;
    MPP_RET ret = p->api->wait(p->ctx, task);

    return ret;
}

MPP_RET mpp_hal_reset(MppHal ctx)
{
    if (NULL == ctx) {
        mpp_err_f("found NULL input\n");
        return MPP_ERR_NULL_PTR;
    }

    MppHalImpl *p = (MppHalImpl*)ctx;
    return p->api->reset(p->ctx);
}

MPP_RET mpp_hal_flush(MppHal ctx)
{
    if (NULL == ctx) {
        mpp_err_f("found NULL input\n");
        return MPP_ERR_NULL_PTR;
    }

    MppHalImpl *p = (MppHalImpl*)ctx;
    return p->api->flush(p->ctx);
}

MPP_RET mpp_hal_control(MppHal ctx, RK_S32 cmd, void *param)
{
    if (NULL == ctx) {
        mpp_err_f("found NULL input\n");
        return MPP_ERR_NULL_PTR;
    }

    MppHalImpl *p = (MppHalImpl*)ctx;
    return p->api->control(p->ctx, cmd, param);
}

HalDeviceId mpp_hal_get_vpu_version()
{
    static HalDeviceId hal_id = HAL_INVALID;
    int32_t fd = -1;
    char temp[256];
    uint32_t i;

    if (hal_id != HAL_INVALID)
        return hal_id;
    
    fd = open ("/proc/device-tree/compatible", O_RDONLY);
    if (fd < 0) {
	/* Assume those kernel without dts supporting use VDPU1 */
        hal_id = HAL_VDPU1;
	return HAL_VDPU1;
    }
    if (read (fd, temp, sizeof(temp) -1) > 0) {
        for (i = 0; i < MPP_VPU_PLATFORM_COUNT; i++) {
	   if (strstr (temp, mpp_vpu_platforms[i].compatible))
	   {
               hal_id = mpp_vpu_platforms[i].hal_id;
	       return hal_id;
	   }
	}
    }
    /*
     * No platform match, maybe not be recorded in the list, assume it
     * use VDPU1
     */
    hal_id = HAL_VDPU1;
    return HAL_VDPU1;
}
