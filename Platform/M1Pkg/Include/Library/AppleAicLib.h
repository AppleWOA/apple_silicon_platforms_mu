/**
 * @file AppleAicLib.h
 * @author amarioguy (Arminder Singh)
 * @brief 
 * 
 * AppleAicLib C header file
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

//AICv1 Registers

#define AIC_V1_BASE FixedPcdGet64(PcdAicV1InterruptControllerBase)

#define AIC_HW_INFO 0x0004

//AICv2 Registers

#define AIC_V2_BASE FixedPcdGet64(PcdAicV2InterruptControllerBase)


#endif //APPLEAIC_H_