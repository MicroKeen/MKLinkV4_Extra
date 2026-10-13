/****************************************************************************
*  Copyright 2024 Gorgon Meducer (Email:embedded_zhuoran@hotmail.com)       *
*                                                                           *
*  Licensed under the Apache License, Version 2.0 (the "License");          *
*  you may not use this file except in compliance with the License.         *
*  You may obtain a copy of the License at                                  *
*                                                                           *
*     http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                           *
*  Unless required by applicable law or agreed to in writing, software      *
*  distributed under the License is distributed on an "AS IS" BASIS,        *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
*  See the License for the specific language governing permissions and      *
*  limitations under the License.                                           *
*                                                                           *
****************************************************************************/

#ifndef __REF_GUI_H__
#define __REF_GUI_H__

/*============================ INCLUDES ======================================*/

#include "ref_gui_cfg.h"

#include "arm_2d_helper.h"
#include "arm_2d_disp_adapters.h"

#include "ngy_message.h"


#ifdef   __cplusplus
extern "C" {
#endif

#if defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wmissing-declarations"
#   pragma clang diagnostic ignored "-Wmicrosoft-anon-tag"
#   pragma clang diagnostic ignored "-Wpadded"
#endif

/*============================ MACROS ========================================*/

/* OOC header, please DO NOT modify  */
#ifdef __REF_GUI_IMPLEMENT__
#   undef   __REF_GUI_IMPLEMENT__
#   define  __ARM_2D_IMPL__
#elif defined(__REF_GUI_INHERIT__)
#   undef   __REF_GUI_INHERIT__
#   define __ARM_2D_INHERIT__
#endif
#include "arm_2d_utils.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/

#define RG_PROCESS_MSG(__GUI_PTR, __ITEM_NAME)                                  \
        arm_using(bool bIsMessageHandled = false)                               \
            for(ngy_msg_core_t *__ITEM_NAME = rg_msg_handling_begin(__GUI_PTR); \
                (__ITEM_NAME != NULL);                                          \
                ({                                                              \
                    rg_msg_handling_end((__GUI_PTR), bIsMessageHandled);        \
                    bIsMessageHandled = false;                                  \
                    __ITEM_NAME = rg_msg_handling_begin(__GUI_PTR);             \
                }))

/*============================ TYPES =========================================*/

typedef struct ref_gui_t ref_gui_t;


typedef struct ref_gui_on_msg_evt_t ref_gui_on_msg_evt_t;

typedef bool rg_msg_handler_t(  ref_gui_t *ptGUI, 
                                void  *pTarget, 
                                ngy_msg_core_t *ptMSG);

struct ref_gui_on_msg_evt_t {
    ref_gui_on_msg_evt_t    *ptNext;

    rg_msg_handler_t        *fnHandler;
    void                    *pTarget;
} ;



typedef struct ref_gui_cfg_t {
    arm_2d_scene_player_t *ptDispAdapter;                                       /* mandatory */
    arm_fsm_rt_t (*fnDispAdapterTask)(void);                                    /* mandatory */

    struct {
        ngy_msg_t       *ptItems;
        uint_fast16_t   hwCount;
    } Message;

    ngy_msg_key_frontend_cfg_t      tKeyFrontend;
    ngy_msg_pointer_frontend_cfg_t  tPointerFrontend;

    ngy_msg_pointer_frontend_pipeline_t *ptPipelines;
    uint_fast8_t chPipelineNo;

    uint8_t bOnlyTheTopSceneCanHandleMessage    : 1;
    uint8_t                                     : 3;
    uint8_t u4MessageBatchProcessingNumber      : 4;


    /*!
     * \brief user defined message preprocessing handler
     * \note this function pointer is OPTIONAL
     * \param[in] ptThis the ref_gui_t object
     * \param[in] ptMSG the target message
     * \retval true skip the system default message handler
     * \retval false use the system default message handler
     */
    bool (*fnUserMessagePreprocessingHandler)(  ref_gui_t *ptThis, 
                                                ngy_msg_core_t *ptMSG);

    /*!
     * \brief user defined default message handler
     * \note this function pointer is OPTIONAL
     * \param[in] ptThis the ref_gui_t object
     * \param[in] ptMSG the target message
     * \retval true skip the system default message handler
     * \retval false use the system default message handler
     */
    bool (*fnUserDefaultMessageHandler)(ref_gui_t *ptThis, 
                                        ngy_msg_core_t *ptMSG);  

} ref_gui_cfg_t;


struct ref_gui_t {
ARM_PRIVATE(
    ref_gui_cfg_t           tCFG;

    ngy_helper_msg_t        tMSGCTRL;

    struct {
        ngy_msg_key_frontend_t  tKey;

        ngy_msg_pointer_frontend_t tPointer;
    } MessageFrontend;

    ngy_helper_msg_fifo_t   tAppMessageFIFO;

    ref_gui_on_msg_evt_t    *ptEventList;

    struct {
        uint8_t bNeedToDrawFrame            : 1;
        uint8_t bConsumeResidualMessage     : 1;
        uint8_t                             : 6;
        int64_t lTimestamp;
    } Runtime;
)
};
extern ref_gui_t g_tMyGUI;
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

extern
ARM_NONNULL(1, 2)
arm_2d_err_t rg_init(ref_gui_t *ptThis, ref_gui_cfg_t *ptCFG);

extern
ARM_NONNULL(1)
arm_fsm_rt_t rg_task(ref_gui_t *ptThis, int_fast8_t chFramerate);

extern
ARM_NONNULL(1)
arm_2d_scene_player_t *rg_get_disp_adapter(ref_gui_t *ptThis);

extern
ARM_NONNULL(1)
ngy_msg_key_frontend_t *rg_get_key_frontend(ref_gui_t *ptThis);

extern
ARM_NONNULL(1)
ngy_msg_pointer_frontend_t *rg_get_pointer_frontend(ref_gui_t *ptThis);

extern
ARM_NONNULL(1)
ngy_msg_t *rg_msg_new(ref_gui_t *ptThis);

extern
ARM_NONNULL(1,2)
void rg_send_msg(ref_gui_t *ptThis,
                 ngy_msg_core_t *ptMSG);

extern
ARM_NONNULL(1)
bool rg_send_pointer_evt(ref_gui_t *ptThis, 
                         uint8_t chPointerEvent,
                         uint16_t hwPointerIndex,
                         int16_t iX,
                         int16_t iY);

extern
ARM_NONNULL(1)
bool rg_send_key_evt(   ref_gui_t *ptThis, 
                        uint8_t chKeyEvent,
                        uint16_t hwKeyValue);

extern
ARM_NONNULL(1)
bool rg_send_gesture_evt(ref_gui_t *ptThis, 
                        uint8_t chGestureEvent,
                        uint8_t chFingerIndex,
                        arm_2d_region_t *ptRegion,
                        uint16_t hwInMs,
                        void *pTargetWidget);
extern
ARM_NONNULL(1,2)
void rg_register_msg_handler(   ref_gui_t *ptThis, 
                                ref_gui_on_msg_evt_t *ptEventHandler);

extern
ARM_NONNULL(1,2)
void rg_unregister_msg_handler( ref_gui_t *ptThis, 
                                ref_gui_on_msg_evt_t *ptEventHandler);
                                
extern
ARM_NONNULL(1)
ngy_msg_core_t *rg_msg_handling_begin(ref_gui_t *ptThis);

extern
ARM_NONNULL(1)
void rg_msg_handling_end(ref_gui_t *ptThis, bool bHandled);


#if defined(__clang__)
#   pragma clang diagnostic pop
#endif

#ifdef   __cplusplus
}
#endif

#endif
