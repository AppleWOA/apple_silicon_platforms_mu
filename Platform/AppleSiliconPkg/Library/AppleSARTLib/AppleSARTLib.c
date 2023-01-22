/**
 * @file AppleSARTLib.c
 * @author amarioguy (Arminder Singh)
 * 
 * The main SART driver library file for Apple silicon platforms.
 * 
 * @version 1.0
 * @date 2022-12-23
 * 
 * @copyright Copyright (c) amarioguy (Arminder Singh), 2022.
 * 
 * SPDX-License-Identifier: MIT
 * 
 * SART hardware definitions and overall driver design philosophy based on m1n1
 * 
 */

#include <PiDxe.h>
#include <ConvenienceMacros.h>
#include <Base.h>
#include <Library/ArmLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>


#define SART_ALLOW_ALL_FLAG 0xff

UINT64 AppleSARTBaseAddress;

