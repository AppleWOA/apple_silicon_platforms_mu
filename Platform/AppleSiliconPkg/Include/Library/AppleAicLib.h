/**
 * @file AppleAicLib.h
 * @author amarioguy (Arminder Singh)
 * @brief 
 * 
 * AppleAicLib C header file. Contains register defines/other things needed by the library
 * 
 * Note - all register defines are relative to AIC_V1_BASE or AIC_V2_BASE
 * 
 * @date 2022-09-05
 * 
 * @copyright Copyright (c) amarioguy (Arminder Singh), 2022.
 * 
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 * 
 */

#ifndef APPLEAIC_H_
#define APPLEAIC_H_

#include <Library/PcdLib.h>
#include <PiPei.h>

/*
 * AIC versions.
 */
typedef enum {
    APPLE_AIC_VERSION_1,
    APPLE_AIC_VERSION_2,
    APPLE_AIC_VERSION_UNKNOWN
} APPLE_AIC_VERSION;


//AICv1 Registers

#define AIC_V1_BASE FixedPcdGet64(PcdAicV1InterruptControllerBase)

#define AIC_V1_HW_INFO 0x0004
//AIC_WHOAMI in m1n1/Linux sources
#define AIC_V1_CPU_IDENTIFIER_REG 0x2000
#define AIC_V1_EVENT_TYPE 0x2004
#define AIC_V1_SEND_IPI_REG 0x2008
#define AIC_V1_ACKNOWLEDGE_IPI_REG 0x200c
//mask/clear IPIs
#define AIC_V1_SET_IPI_MASK_REG 0x2024
#define AIC_V1_CLEAR_IPI_MASK_REG 0x2028






//AICv1 Event Types
//which CPU die did this occur on?
#define AIC_V1_EVENT_WHICH_CPU_DIE 0xFF000000
//are we an FIQ, IRQ, or IPI?
#define AIC_V1_EVENT_INTERRUPT_TYPE 0xFF0000


//AICv2 Registers


//AIC IMPDEF system registers

#define AIC_V2_BASE FixedPcdGet64(PcdAicV2InterruptControllerBase)


#endif //APPLEAIC_H_