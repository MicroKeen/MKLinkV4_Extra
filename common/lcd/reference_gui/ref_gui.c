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


/*============================ INCLUDES ======================================*/

#define __NGY_MESSAGE_INHERIT__
#define __REF_GUI_IMPLEMENT__
#include "ref_gui.h"

#if defined(__clang__)
//#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wunknown-warning-option"
#   pragma clang diagnostic ignored "-Wreserved-identifier"
#   pragma clang diagnostic ignored "-Wsign-conversion"
#   pragma clang diagnostic ignored "-Wpadded"
#   pragma clang diagnostic ignored "-Wcast-qual"
#   pragma clang diagnostic ignored "-Wcast-align"
#   pragma clang diagnostic ignored "-Wmissing-field-initializers"
#   pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#   pragma clang diagnostic ignored "-Wmissing-prototypes"
#   pragma clang diagnostic ignored "-Wunused-variable"
#   pragma clang diagnostic ignored "-Wunused-parameter"
#   pragma clang diagnostic ignored "-Wgnu-statement-expression"
#   pragma clang diagnostic ignored "-Wtautological-pointer-compare"
#elif __IS_COMPILER_ARM_COMPILER_5__
#elif __IS_COMPILER_GCC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wformat="
#   pragma GCC diagnostic ignored "-Wpedantic"
#endif

/*============================ MACROS ========================================*/
#undef this
#define this    (*ptThis)
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

static
void __rg_before_scene_deposing_handler(void *pTarget,
                                        arm_2d_scene_player_t *ptPlayer,
                                        arm_2d_scene_t *ptScene);

/*============================ LOCAL VARIABLES ===============================*/
/*============================ IMPLEMENTATION ================================*/

ARM_NONNULL(1, 2)
arm_2d_err_t rg_init(ref_gui_t *ptThis, ref_gui_cfg_t *ptCFG)
{
    assert(NULL != ptThis);
    assert(NULL != ptCFG);

    ARM_2D_LOG_INFO(
        GUI_STACK,
        0,
        "Reference GUI",
        "Initialize Reference GUI [%p]...",
        ptThis
    );

    memset(ptThis, 0, sizeof(ref_gui_t));

    this.tCFG = *ptCFG;
    if (NULL == this.tCFG.ptDispAdapter) {
        ARM_2D_LOG_ERROR(
            GUI_STACK,
            1,
            "Reference GUI",
            "The .ptDispAdapter is NULL\r\n"
        );
        return ARM_2D_ERR_MISSING_PARAM;
    }

    if (NULL == this.tCFG.fnDispAdapterTask) {
        ARM_2D_LOG_ERROR(
            GUI_STACK,
            1,
            "Reference GUI",
            "The .fnDispAdapterTask is NULL\r\n"
        );
        return ARM_2D_ERR_MISSING_PARAM;
    }

    /* initialize the message helper */
    do {
        ngy_helper_msg_cfg_t tCFG = {
            .bAllowUsingHeap = true,
        };
        ngy_helper_msg_init(&this.tMSGCTRL, &tCFG);
    } while(0);

    if (    (NULL != this.tCFG.Message.ptItems)
      &&    (this.tCFG.Message.hwCount > 0)) {
        ngy_helper_msg_add_items_to_pool(   &this.tMSGCTRL,
                                            this.tCFG.Message.ptItems,
                                            this.tCFG.Message.hwCount);
    }

    /* initialize user message fifo */
    ngy_helper_msg_fifo_init(&this.tAppMessageFIFO);

    /* initialize message frontend */
    do {

        ngy_helper_msg_key_frontend_init(   &this.MessageFrontend.tKey, 
                                            &this.tCFG.tKeyFrontend);

        ngy_helper_msg_frontend_register(&this.tMSGCTRL,
                                         &this.MessageFrontend.tKey.use_as__ngy_msg_frontend_t,
                                         NGY_FRONTEND_KEY);

        ngy_helper_msg_pointer_frontend_init(&this.MessageFrontend.tPointer, 
                                             &this.tCFG.tPointerFrontend,
                                             this.tCFG.ptPipelines,
                                             this.tCFG.chPipelineNo);
        
        ngy_helper_msg_frontend_register(&this.tMSGCTRL,
                                         &this.MessageFrontend.tPointer.use_as__ngy_msg_frontend_t,
                                         NGY_FRONTEND_POINTER);

    } while(0);

    if (0 == this.tCFG.u4MessageBatchProcessingNumber) {
        this.tCFG.u4MessageBatchProcessingNumber = 8;
    }

    /* register before scene deposing event handler */
    arm_2d_scene_player_register_before_deposing_event_handler(
        this.tCFG.ptDispAdapter,
        __rg_before_scene_deposing_handler,
        ptThis);

    ARM_2D_LOG_INFO(
        GUI_STACK,
        0,
        "Reference GUI",
        "OK\r\n"
    );

    return ARM_2D_ERR_NONE;
}

ARM_NONNULL(1)
ngy_msg_key_frontend_t *rg_get_key_frontend(ref_gui_t *ptThis)
{
    assert(NULL != ptThis);

    return &this.MessageFrontend.tKey;
}

ARM_NONNULL(1)
ngy_msg_pointer_frontend_t *rg_get_pointer_frontend(ref_gui_t *ptThis)
{
    assert(NULL != ptThis);

    return &this.MessageFrontend.tPointer;
}

ARM_NONNULL(1)
arm_2d_scene_player_t *rg_get_disp_adapter(ref_gui_t *ptThis)
{
    assert(ptThis);
    return this.tCFG.ptDispAdapter;
}

static
void __rg_before_scene_deposing_handler(void *pTarget,
                                        arm_2d_scene_player_t *ptPlayer,
                                        arm_2d_scene_t *ptScene)
{
    ref_gui_t *ptThis = (ref_gui_t *)pTarget;

    ref_gui_on_msg_evt_t **pptItem = &this.ptEventList;
    bool bFoundItem = false;

    while(NULL != *pptItem) {
        arm_irq_safe {
            if ((*pptItem)->pTarget == (void *)ptScene) {
                /* remove this node */
                ref_gui_on_msg_evt_t *ptTargetItem = (*pptItem);
                (*pptItem) = (*pptItem)->ptNext;
                ptTargetItem->ptNext = NULL;
                bFoundItem = true;
            }
        }

        if (bFoundItem) {
            return ;
        }

        pptItem = &((*pptItem)->ptNext);
    }
}

ARM_NONNULL(1,2)
void rg_register_msg_handler(   ref_gui_t *ptThis, 
                                ref_gui_on_msg_evt_t *ptEventHandler)
{
    assert(NULL != ptThis);
    assert(NULL != ptEventHandler);

    ref_gui_on_msg_evt_t *ptItem = this.ptEventList;

    while(NULL != ptItem) {
        if (ptItem == ptEventHandler) {
            return ;
        }

        ptItem = ptItem->ptNext;
    }

    arm_irq_safe {
        /* add event handler */
        __ARM_LIST_STACK_PUSH(this.ptEventList, ptEventHandler);
    }

}

ARM_NONNULL(1,2)
void rg_unregister_msg_handler( ref_gui_t *ptThis, 
                                ref_gui_on_msg_evt_t *ptEventHandler)
{
    assert(NULL != ptThis);
    assert(NULL != ptEventHandler);

    ref_gui_on_msg_evt_t **pptItem = &this.ptEventList;
    bool bFoundItem = false;

    while(NULL != *pptItem) {
        
        arm_irq_safe {
            if ((*pptItem) == ptEventHandler) {
                /* remove this node */
                ref_gui_on_msg_evt_t *ptTargetItem = (*pptItem);
                (*pptItem) = (*pptItem)->ptNext;
                ptTargetItem->ptNext = NULL;
                bFoundItem = true;
            }
        }

        if (bFoundItem) {
            return ;
        }

        pptItem = &((*pptItem)->ptNext);
    }
}



static 
bool __rg_manual_scene_switching(ref_gui_t *ptThis, ngy_msg_core_t *ptMSG)
{
    /* todo */
    return true;
}

__WEAK void __rg_default_message_handler(ref_gui_t *ptThis, ngy_msg_core_t *ptMSG)
{
    assert(ptThis);

    if (NULL == ptMSG) {
        return ;
    }

    do {
        if (ARM_2D_INVOKE( this.tCFG.fnUserDefaultMessageHandler,
                            ARM_2D_PARAM(
                                ptThis,
                                ptMSG
                            ))) {
            /* the user has handled the message and wants to 
            * ignore the default message handler 
            */
            break;
        }

        /* todo: other default message handling */


    } while(0);

    ngy_helper_msg_free(&this.tMSGCTRL, (ngy_msg_t *)ptMSG);
}

__WEAK
void __rg_default_message_preprocessing_handler(ref_gui_t *ptThis)
{
    assert(ptThis);

    /* message filter */
    int_fast8_t nMessageBatchProcessCount = this.tCFG.u4MessageBatchProcessingNumber;

    do {
        /* get a message */
        ngy_msg_core_t *ptMSG = ngy_helper_msg_get(&this.tMSGCTRL);

        if (NULL == ptMSG) {
            return ;
        }

        
        do {
            if (ARM_2D_INVOKE( this.tCFG.fnUserMessagePreprocessingHandler,
                                ARM_2D_PARAM(
                                    ptThis,
                                    ptMSG
                                ))) {
                /* the user has handled the message and wants to 
                * ignore the default message handler 
                */
                ngy_helper_msg_free(&this.tMSGCTRL, (ngy_msg_t *)ptMSG);
                break;
            }

            if (arm_2d_scene_player_is_switching(this.tCFG.ptDispAdapter)) {
                if (__rg_manual_scene_switching(ptThis, ptMSG)) {
                    ngy_helper_msg_free(&this.tMSGCTRL, (ngy_msg_t *)ptMSG);
                    break;
                }
            }

            /* todo : other default message handling 
            if (...) {
                ngy_helper_msg_free(&this.tMSGCTRL, (ngy_msg_t *)ptMSG);
                break;
            }
            */

            /* call message handler */
            if (NULL != this.ptEventList) {

                bool bMessageHandled = false;
                ref_gui_on_msg_evt_t *ptItem = this.ptEventList;

                do {
                    /* call message handler */
                    if (ARM_2D_INVOKE(ptItem->fnHandler,
                            ARM_2D_PARAM(ptThis, ptItem->pTarget,ptMSG))) {
                        bMessageHandled = true;
                        break;
                    }

                    if (this.tCFG.bOnlyTheTopSceneCanHandleMessage) {
                        break;
                    }

                    ptItem = ptItem->ptNext;
                } while(ptItem != NULL);
                
                if (bMessageHandled) {
                    ngy_helper_msg_free(&this.tMSGCTRL, (ngy_msg_t *)ptMSG);
                } else {
                    __rg_default_message_handler(ptThis, ptMSG);
                }

                break;
            }

            /* add message to the App message FIFO */
            ngy_helper_msg_fifo_append(&this.tAppMessageFIFO, ptMSG);
            
        } while(0);
    } while(--nMessageBatchProcessCount);
    
}

ARM_NONNULL(1)
arm_fsm_rt_t rg_task(ref_gui_t *ptThis, int_fast8_t chFramerate) 
{
    assert(NULL != ptThis);
    assert(NULL != this.tCFG.fnDispAdapterTask);

    if (NULL == this.tCFG.fnDispAdapterTask) {
        return (arm_fsm_rt_t)ARM_2D_ERR_INVALID_PARAM;
    }

    arm_fsm_rt_t tResult = arm_fsm_rt_on_going;

    if (chFramerate == 0) {
        /* no display */
        this.Runtime.bNeedToDrawFrame = false;
        tResult = arm_fsm_rt_cpl;
        /* avoid message congestion */
        this.Runtime.bConsumeResidualMessage = true;
    } else if (chFramerate < 0) {
        /* no framerate locking */
        tResult = ARM_2D_INVOKE(this.tCFG.fnDispAdapterTask);
        this.Runtime.bNeedToDrawFrame = !(tResult == arm_fsm_rt_cpl);
        if (!this.Runtime.bNeedToDrawFrame) {
            this.Runtime.bConsumeResidualMessage = true;
        }
    } else if (this.Runtime.bNeedToDrawFrame) {
        tResult = ARM_2D_INVOKE(this.tCFG.fnDispAdapterTask);
        if (arm_fsm_rt_cpl == tResult) {
            this.Runtime.bNeedToDrawFrame = false;
            this.Runtime.bConsumeResidualMessage = true;
        }
    } else if (arm_2d_helper_is_time_out(1000 / chFramerate, &this.Runtime.lTimestamp)) {
        this.Runtime.bNeedToDrawFrame = true;
    }

    /* consume the leftover in AppMessageFIFO to avoid message congestion */
    if (this.Runtime.bConsumeResidualMessage) {
        this.Runtime.bConsumeResidualMessage = false;

        do {
            ngy_msg_core_t *ptMSG = ngy_helper_msg_fifo_get(&this.tAppMessageFIFO);
            if (NULL == ptMSG) {
                break;
            }
            __rg_default_message_handler(ptThis, ptMSG);
        } while(true);
    }
    
    /* preprocess messages */
    while (arm_fsm_rt_on_going == ngy_helper_msg_task(&this.tMSGCTRL));


    do {
    #if 0
        if (!arm_2d_scene_player_is_switching(this.tCFG.ptDispAdapter)) {
            break;
        }
    #endif
        /* NOTE: When bNeedToDrawFrame is true, we are sure it is between two frames, and
        *       it is safe to do any updates on scene content. 
        */
        if (!this.Runtime.bNeedToDrawFrame) {
            /* safe to handle message */
            __rg_default_message_preprocessing_handler(ptThis);
        }
        
    } while(0);

    return tResult;

}

ARM_NONNULL(1,2)
void rg_send_msg(ref_gui_t *ptThis,
                 ngy_msg_core_t *ptMSG)
{
    assert(NULL != ptThis);
    assert(NULL != ptMSG);

    ngy_helper_msg_send(&this.tMSGCTRL, ptMSG);
}

ARM_NONNULL(1)
ngy_msg_t *rg_msg_new(ref_gui_t *ptThis)
{
    assert(NULL != ptThis);

    return ngy_helper_msg_new(&this.tMSGCTRL);
}

ARM_NONNULL(1)
bool rg_send_pointer_evt(ref_gui_t *ptThis, 
                         uint8_t chPointerEvent,
                         uint16_t hwPointerIndex,
                         int16_t iX,
                         int16_t iY)
{
    assert(NULL != ptThis);

    return ngy_helper_msg_send_pointer_event(   &this.tMSGCTRL,
                                                chPointerEvent,
                                                hwPointerIndex,
                                                iX,
                                                iY,
                                                NULL);
}



ARM_NONNULL(1)
bool rg_send_key_evt(   ref_gui_t *ptThis, 
                        uint8_t chKeyEvent,
                        uint16_t hwKeyValue)
{
    assert(NULL != ptThis);

    return ngy_helper_msg_send_key_event(   &this.tMSGCTRL,
                                            chKeyEvent,
                                            hwKeyValue,
                                            NULL);
}

ARM_NONNULL(1)
bool rg_send_gesture_evt(ref_gui_t *ptThis, 
                        uint8_t chGestureEvent,
                        uint8_t chFingerIndex,
                        arm_2d_region_t *ptRegion,
                        uint16_t hwInMs,
                        void *pTargetWidget)
{
    assert(NULL != ptThis);

    return ngy_helper_msg_send_gesture_event(   &this.tMSGCTRL,
                                                chGestureEvent,
                                                chFingerIndex,
                                                ptRegion,
                                                hwInMs,
                                                NULL);
}

ARM_NONNULL(1)
ngy_msg_core_t *rg_msg_handling_begin(ref_gui_t *ptThis)
{
    assert(NULL != ptThis);

    return ngy_helper_msg_fifo_peek(&this.tAppMessageFIFO);
}

ARM_NONNULL(1)
void rg_msg_handling_end(ref_gui_t *ptThis, bool bHandled)
{
    assert(NULL != ptThis);

    ngy_msg_core_t *ptMSG = ngy_helper_msg_fifo_get(&this.tAppMessageFIFO);

    if (bHandled) {
        ngy_helper_msg_free( &this.tMSGCTRL, (ngy_msg_t *)ptMSG);
        return ;
    }

    __rg_default_message_handler(ptThis, ptMSG);
}


