/**
 * @file AppleAicV2Dxe.c
 * amarioguy (Arminder Singh)
 * 
 * AICv2 specific DXE initialization/driver code.
 * 
 * @version 1.0
 * @date 2022-09-29
 * 
 * @copyright Copyright (c) amarioguy (Arminder Singh), 2022.
 * 
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 * 
 */

#include <Library/AppleAicLib.h>
#include "AppleAicDxe.h"

STATIC UINT64 AicV2Base;

extern EFI_HARDWARE_INTERRUPT_PROTOCOL   gHardwareInterruptAicV2Protocol;
extern EFI_HARDWARE_INTERRUPT2_PROTOCOL  gHardwareInterrupt2AicV2Protocol;


STATIC EFI_STATUS EFIAPI AppleAicV2MaskInterrupt(
    IN EFI_HARDWARE_INTERRUPT_PROTOCOL *This,
    IN HARDWARE_INTERRUPT_SOURCE Source
)
{
    //right now this value is hardcoded to max IRQ amount supported on M1v2/M1 Ultra
    //TODO: don't hardcode this
    if(Source >= 4096)
    {
        DEBUG((DEBUG_INFO, "%a: Cannot mask IRQ higher than maximum supported IRQ!", __FUNCTION__));
        ASSERT(FALSE);
        return EFI_UNSUPPORTED;
    }

    AppleAicMaskInterrupt(AicV2Base, Source);
    return EFI_SUCCESS;
}

/**
 * Prepares the AIC protocol for use by the DXE environment.
 * 
 * @param ImageHandle
 * @param SystemTable 
 * @return EFI_STATUS 
 */

EFI_STATUS AppleAicV2DxeInit(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
    UINTN InterruptIndex;
    EFI_STATUS Status;
    UINT32 AicV2NumInterrupts;
    //Assert that the Hardware Interrupt protocol is not installed already.
    ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gHardwareInterruptProtocolGuid);

    AicV2Base = PcdGet64(PcdAicInterruptControllerBase);
    AicV2NumInterrupts = AppleAicGetNumInterrupts();
    DEBUG((DEBUG_INFO, "AICv2 with %d configured IRQs", AicV2NumInterrupts));

    //start from a clean state by disabling all interrupts
    for(InterruptIndex = 0; InterruptIndex < AicV2NumInterrupts; InterruptIndex++)
    {
        AppleAicV2MaskInterrupt(&gHardwareInterruptAicV2Protocol, InterruptIndex);
    }
}