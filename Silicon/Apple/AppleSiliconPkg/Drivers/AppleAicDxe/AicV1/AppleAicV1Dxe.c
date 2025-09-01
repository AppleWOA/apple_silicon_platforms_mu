/**
 * Copyright (c) 2025, AppleWOA authors.
 * 
 * Module Name:
 *     AppleAicV1Dxe.c
 * 
 * Abstract:
 *     AICv1 interrupt controller initialization and driver code.
 * 
 * 
 * Environment:
 *     UEFI DXE (Driver Execution Environment).
 * 
 * License:
 *     SPDX-License-Identifier: (BSD-2-Clause-Patent OR MIT) AND GPL-2.0
 * 
 *     Original code basis is from the Asahi Linux project forks of u-boot and Linux, as well as m1n1. 
 *     Original copyright and author notices below.
 *     Copyright (C) 2021 Mark Kettenis <kettenis@openbsd.org>
 *     Copyright (C) The Asahi Linux Contributors.
*/

#include <Library/AppleAicLib.h>
#include "AppleAicDxe.h"
#include <Library/AppleSysRegs.h>
#include <Library/ArmGenericTimerCounterLib.h>
#include <Library/AppleDTLib.h>

//
// AICv1 has a lot of the same semantic operations as AICv2, having separate set/clear registers
// for interrupts, the critical difference is really in how interrupt routing is handled,
// with AICv1 using traditional interrupt affinity routing, along with having support for "slow" IPIs via
// the AIC itself (making it closer to a traditional GIC in that sense.)
// 
// The plan for this firmware is to rely on AIC-based IPIs over Fast IPIs on AICv1 platforms.
// (less FIQs to handle is always a benefit in the Windows case...)

#define AIC_V1_REG_SIZE 0x8000
#define AIC_V1_EVENT_OFFSET 0x2004
#define AIC_V1_TARGET_CPU 0x3000
#define AIC_V1_SW_SET 0x4000
#define AIC_V1_SW_CLR 0x4080
#define AIC_V1_MASK_SET 0x4100
#define AIC_V1_MASK_CLR 0x4180

STATIC UINT64 AicBase, mAicEventRegOffset, mAicSoftwareSetRegOffset;
STATIC UINT64 mAicTargetCpuRegOffset, mAicRegSizeOffset, mAicSoftwareClearRegOffset, mAicMaskSetRegOffset, mAicMaskClearRegOffset;

extern EFI_HARDWARE_INTERRUPT_PROTOCOL   gHardwareInterruptAicV1Protocol;
extern EFI_HARDWARE_INTERRUPT2_PROTOCOL  gHardwareInterrupt2AicV1Protocol;


AIC_INFO_STRUCT *AicInfoStruct;

EFI_STATUS AppleAicV1DxeInit(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) 
{
    AicInfoStruct = AllocatePool(sizeof(AIC_INFO_STRUCT));
    //
    // Check that the Hardware Interrupt protocol isn't already installed.
    //
    ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gHardwareInterruptProtocolGuid);

    //
    // AICv1 is a much simpler setup here compared to AICv2 and AICv3, since
    // since a bunch of device dependent registers are static offsets from
    // AIC base and we won't need to deal with multi-die on AICv1 devices ever.
    //
    dt_node_t *InterruptControllerNode = dt_get("aic");
    if (!InterruptControllerNode) {
        DEBUG((EFI_D_INFO | EFI_D_LOAD | EFI_D_ERROR, "no ADT supplied, exiting\n"));
        ASSERT(FALSE);
    }
    dt_node_reg(InterruptControllerNode, 0, &AicBase, NULL);

    AicInfoStruct->NumIrqs = AppleAicGetNumInterrupts(AicBase);
    AicInfoStruct->MaxIrqs = AppleAicGetMaxInterrupts(AicBase);

    AicInfoStruct->MaxCpuDies = 1;
    AicInfoStruct->NumCpuDies = 1;

    mAicEventRegOffset = 0;
    mAicMaskClearRegOffset = AIC_V1_MASK_CLR;
    mAicMaskSetRegOffset = AIC_V1_MASK_SET;
    mAicSoftwareClearRegOffset = AIC_V1_SW_CLR;
    mAicSoftwareSetRegOffset = AIC_V1_SW_SET;
    //
    // This seems equivalent to the IRQ_CFG register in AICv2?
    //
    mAicTargetCpuRegOffset = AIC_V1_TARGET_CPU;
    mAicRegSizeOffset = AIC_V1_REG_SIZE;

    DEBUG((DEBUG_INFO, "AICv1 with %u/%u configured IRQs, at 0x%llx\n", AicInfoStruct->NumIrqs, AicInfoStruct->MaxIrqs, AicBase));
    DEBUG((DEBUG_VERBOSE, "AIC register addresses: \n"
    "\tAIC_V1_BASE: 0x%llx\n"
    "\tAIC_V1_SW_SET: 0x%llx\n"
    "\tAIC_V1_SW_CLR: 0x%llx\n"
    "\tAIC_V1_MASK_SET: 0x%llx\n"
    "\tAIC_V1_MASK_CLR: 0x%llx\n"
    "\tAIC_V1_EVENT: 0x%llx\n",
    AicBase,
    (AicBase + mAicSoftwareSetRegOffset),
    (AicBase + mAicSoftwareClearRegOffset),
    (AicBase + mAicMaskSetRegOffset),
    (AicBase + mAicMaskClearRegOffset),
    (AicBase + mAicEventRegOffset)
    ));

    //
    // Turn on the AIC.
    //
    // TODO: is AICv1 always on?
    // DEBUG((DEBUG_VERBOSE, "%a: enabling AIC\n", __FUNCTION__));


    //
    // TODO: Disable all interrupts for now.
    //
    // Also of note: AICv1 uses traditional MPIDR based affinity routing instead of
    // the hardware based heuristic used on AICv2 and AICv3.
    //
    //register the interrupt controller now that setup is done.

    // Status = InstallAndRegisterInterruptService(
    //     &gHardwareInterruptAicV1Protocol,
    //     &gHardwareInterrupt2AicV1Protocol,
    //     AppleAicV1InterruptHandler,
    //     AppleAicV1ExitBootServicesEvent
    // );
    return EFI_SUCCESS;
}
