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
#include <Include/libfdt.h>
#include "AppleAicDxe.h"

STATIC UINT64 AicV2Base;
AIC_INFO_STRUCT *AicInfoStruct;

extern EFI_HARDWARE_INTERRUPT_PROTOCOL   gHardwareInterruptAicV2Protocol;
extern EFI_HARDWARE_INTERRUPT2_PROTOCOL  gHardwareInterrupt2AicV2Protocol;


STATIC EFI_STATUS EFIAPI AppleAicV2MaskInterrupt(
    IN EFI_HARDWARE_INTERRUPT_PROTOCOL *This,
    IN HARDWARE_INTERRUPT_SOURCE Source
)
{
    UINT32 MaxIrqs = AppleAicGetMaxInterrupts(AicV2Base);
    if(Source >= MaxIrqs)
    {
        DEBUG((DEBUG_INFO, "%a: Cannot mask IRQ higher than maximum supported IRQ!\n", __FUNCTION__));
        ASSERT(FALSE);
        return EFI_UNSUPPORTED;
    }

    AppleAicMaskInterrupt(AicV2Base, Source);
    return EFI_SUCCESS;
}

/**
 * Calculate the AIC register offsets on the platform
 * 
 * @return EFI_SUCCESS unconditionally.
 */
EFI_STATUS AppleAicV2CalculateRegisterOffsets(IN VOID)
{
    //needed for devicetree calculations
    VOID* FdtBlob = (VOID *)(PcdGet64(PcdFdtPointer));
    UINT32 InterruptControllerNode;
    UINT32 Length;
    UINT32 AddressCells;
    UINT32 SizeCells;
    UINT64 StartOffset;
    UINT64 CurrentOffset;

    //Start with simple things
    AicV2Base = PcdGet64(PcdAicInterruptControllerBase);
    AicInfoStruct->NumIrqs = AppleAicGetNumInterrupts(AicV2Base);
    AicInfoStruct->MaxIrqs = AppleAicGetMaxInterrupts(AicV2Base);
    AicInfoStruct->MaxCpuDies = FIELD_GET(AIC_V2_INFO_REG3_MAX_DIE_COUNT_BITFIELD, MmioRead32(AicV2Base + AIC_V2_INFO_REG3));
    AicInfoStruct->NumCpuDies = (FIELD_GET(AIC_V2_INFO_REG1_LAST_CPU_DIE_BITFIELD, MmioRead32(AicV2Base + AIC_V2_INFO_REG1))) + 1;
    
    //Now for the more complicated bits...
    //Start by getting the event register offset from the devicetree
    InterruptControllerNode = fdt_path_offset(FdtBlob, "/soc/interrupt-controller");
    AicInfoStruct->Regs.EventReg = fdt_getprop(FdtBlob, InterruptControllerNode, "event", Length);
    if(Length < (AddressCells + SizeCells) * sizeof(UINT32)) {
        DEBUG((DEBUG_ERROR, "%a: Failed to get event register offset from DeviceTree!\n", __FUNCTION__));
        return EFI_ERROR(-1);
    }
    //Now calculate everything else, starting from the IRQ_CFG register
    StartOffset = AicInfoStruct->Regs.IrqConfigRegOffset = (AicV2Base + AIC_V2_IRQ_CFG_REG);
    CurrentOffset = (StartOffset + sizeof(UINT32) * AicInfoStruct->MaxIrqs);

    AicInfoStruct->Regs.SoftwareSetRegOffset = CurrentOffset;
    CurrentOffset += sizeof(UINT32) * ((AicInfoStruct->MaxIrqs) >> 5);
    AicInfoStruct->Regs.SoftwareClearRegOffset = CurrentOffset;
    CurrentOffset += sizeof(UINT32) * ((AicInfoStruct->MaxIrqs) >> 5);
    AicInfoStruct->Regs.IrqMaskSetRegOffset = CurrentOffset;
    CurrentOffset += sizeof(UINT32) * ((AicInfoStruct->MaxIrqs) >> 5);
    AicInfoStruct->Regs.IrqMaskClearRegOffset = CurrentOffset;
    CurrentOffset += sizeof(UINT32) * ((AicInfoStruct->MaxIrqs) >> 5);
    AicInfoStruct->Regs.HwStateRegOffset = CurrentOffset;
    AicInfoStruct->DieStride = CurrentOffset - StartOffset;
    AicInfoStruct->RegSize = AicInfoStruct->Regs.EventReg + 4;
    
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
    UINT32 AicV2MaxInterrupts;
    DEBUG((DEBUG_INFO, "%a: AIC driver start\n", __FUNCTION__));
    //Assert that the Hardware Interrupt protocol is not installed already.
    ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gHardwareInterruptProtocolGuid);

    //set up and collect variables in a global struct that holds AIC register offsets and useful values.
    //this is the approach m1n1 and Linux use
    Status = AppleAicV2CalculateRegisterOffsets();
    AicV2NumInterrupts = AicInfoStruct->NumIrqs;
    AicV2MaxInterrupts = AicInfoStruct->MaxIrqs;
    
    DEBUG((DEBUG_INFO, "AICv2 with %u/%u configured IRQs, at 0x%llx\n", AicV2NumInterrupts, AicV2MaxInterrupts, AicV2Base));
    DEBUG((DEBUG_INFO, "%u/%u CPU dies, Die Stride: 0x%lx", AicInfoStruct->NumCpuDies, AicInfoStruct->MaxCpuDies, AicInfoStruct->DieStride));
    DEBUG((DEBUG_VERBOSE, "AIC register addresses: \n"
    "\tAIC_V2_BASE: 0x%llx\n"
    "\tAIC_V2_IRQ_CFG: 0x%llx\n"
    "\tAIC_V2_SW_SET: 0x%llx\n"
    "\tAIC_V2_SW_CLR: 0x%llx\n"
    "\tAIC_V2_MASK_SET: 0x%llx\n"
    "\tAIC_V2_MASK_CLR: 0x%llx\n"
    "\tAIC_V2_HW_STATE: 0x%llx\n"
    "\tAIC_V2_EVENT: 0x%llx\n",
    AicV2Base,
    (AicV2Base + AicInfoStruct->Regs.IrqConfigRegOffset),
    (AicV2Base + AicInfoStruct->Regs.SoftwareSetRegOffset),
    (AicV2Base + AicInfoStruct->Regs.SoftwareClearRegOffset),
    (AicV2Base + AicInfoStruct->Regs.IrqMaskSetRegOffset),
    (AicV2Base + AicInfoStruct->Regs.IrqMaskClearRegOffset),
    (AicV2Base + AicInfoStruct->Regs.HwStateRegOffset),
    (AicInfoStruct->Regs.EventReg)
    ));

    //start from a clean state by disabling all interrupts
    for(InterruptIndex = 0; InterruptIndex < AicV2NumInterrupts; InterruptIndex++)
    {
        AppleAicV2MaskInterrupt(&gHardwareInterruptAicV2Protocol, InterruptIndex);
    }
    MmioAnd32(AicV2Base + AIC_V2_CONFIG, ~(1));
}