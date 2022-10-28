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

#define APPLE_FAST_IPI_STATUS_PENDING BIT(0)

STATIC UINT64 AicV2Base;
AIC_INFO_STRUCT *AicInfoStruct;

extern EFI_HARDWARE_INTERRUPT_PROTOCOL   gHardwareInterruptAicV2Protocol;
extern EFI_HARDWARE_INTERRUPT2_PROTOCOL  gHardwareInterrupt2AicV2Protocol;

/**
 * Masks an IRQ on an AICv2 platform.
 * 
 * @param This - pointer to interrupt protocol instance
 * @param Source - IRQ number.
 * @return EFI_SUCCESS if successful, ASSERTs if source is > than max IRQs
 */

STATIC EFI_STATUS EFIAPI AppleAicV2MaskInterrupt(
    IN EFI_HARDWARE_INTERRUPT_PROTOCOL *This,
    IN HARDWARE_INTERRUPT_SOURCE Source
)
{
    if(Source >= AicInfoStruct->MaxIrqs)
    {
        DEBUG((DEBUG_INFO, "%a: Cannot mask IRQ higher than maximum supported IRQ!\n", __FUNCTION__));
        ASSERT(FALSE);
        return EFI_UNSUPPORTED;
    }

    AppleAicMaskInterrupt(AicV2Base, Source);
    return EFI_SUCCESS;
}

/**
 * Unmasks an IRQ on an AICv2 platform.
 * 
 * @param This - pointer to interrupt protocol instance
 * @param Source - IRQ number.
 * @return EFI_SUCCESS if successful, ASSERTs if source is > than max IRQs
 */

STATIC EFI_STATUS EFIAPI AppleAicV2UnmaskInterrupt(
    IN EFI_HARDWARE_INTERRUPT_PROTOCOL *This,
    IN HARDWARE_INTERRUPT_SOURCE Source
)
{
    if(Source >= AicInfoStruct->MaxIrqs)
    {
        DEBUG((DEBUG_INFO, "%a: Cannot unmask IRQ higher than maximum supported IRQ!\n", __FUNCTION__));
        ASSERT(FALSE);
        return EFI_UNSUPPORTED;
    }

    AppleAicUnmaskInterrupt(AicV2Base, Source);
    return EFI_SUCCESS;
}

/**
 * Gets the state of a particular interrupt on the platform.
 * 
 * TODO: figure out if we need to read HW_STATE or IRQ_CFG to read the state of interrupts
 * 
 * @param This - Interrupt protocol instance pointer
 * @param Source - IRQ number
 * @param State - pointer to BOOLEAN that holds the actual interrupt state. TRUE if enabled, FALSE if disabled.
 * @return EFI_SUCCESS if successful, ASSERTs if Source >= MAX_IRQs 
 */
STATIC EFI_STATUS EFIAPI AppleAicV2GetInterruptState(
    IN EFI_HARDWARE_INTERRUPT_PROTOCOL *This,
    IN HARDWARE_INTERRUPT_SOURCE Source,
    OUT BOOLEAN *State
)
{
    if(Source >= AicInfoStruct->MaxIrqs)
    {
        ASSERT(FALSE);
        return EFI_UNSUPPORTED;
    }
    *State = AppleAicReadInterruptState(AicV2Base, Source);
    return EFI_SUCCESS;
}



/**
 * Sends the EOI signal to hardware.
 * 
 * @param This - interrupt protocol instance pointer
 * @param Source - IRQ number
 * @return EFI_SUCCESS if successful.
 */
STATIC EFI_STATUS EFIAPI AppleAicV2EndOfInterrupt(
    IN EFI_HARDWARE_INTERRUPT_PROTOCOL *This,
    IN HARDWARE_INTERRUPT_SOURCE Source
)
{
    //TODO: FIQ case

    //reading the interrupt source in the event register acks and masks it at the same time
    //all we need to do is unmask it here. (note that for hardware IRQs, this assumes the hardware interrupt source has been cleared)
    AppleAicUnmaskInterrupt(This, Source);
}


/**
 * Calculate the AIC register offsets on the platform.
 * 
 * @return EFI_SUCCESS if successful, EFI_ERROR(-1) if an error occurred.
 */
EFI_STATUS EFIAPI AppleAicV2CalculateRegisterOffsets(IN VOID)
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
    AicInfoStruct->Regs.EventReg = fdt_getprop(FdtBlob, InterruptControllerNode, "reg", Length);
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
    AicInfoStruct->RegSize = (AicInfoStruct->Regs.EventReg - AicV2Base) + 4;
    
    return EFI_SUCCESS;

}

/**
 * EFI_CPU_INTERRUPT_HANDLER entered when a processor interrupt is taken.
 * 
 * Note that this handler handles both FIQs and IRQs.
 * 
 * @param InterruptType - type of interrupt taken. will be EXCEPT_AARCH64_{IRQ, FIQ, SERROR} for IRQs, FIQs, and SErrors respectively.
 * @param SystemContext - CPU context snapshot when interrupt was taken
 * @return Nothing. 
 */
STATIC VOID EFIAPI AppleAicV2InterruptHandler(
    IN EFI_EXCEPTION_TYPE InterruptType,
    IN EFI_SYSTEM_CONTEXT SystemContext
)
{
    UINT32 AicInterrupt;
    HARDWARE_INTERRUPT_HANDLER HwInterruptHandler;
    UINT64 PmcStatus;
    UINT64 UncorePmcStatus;

    AicInterrupt = AppleAicAcknowledgeInterrupt();
    HwInterruptHandler = AicRegisteredInterruptHandlers[AicInterrupt];

    /**
     * In the FIQ case, every possible FIQ source must be checked to avoid an interrupt storm.
     * (Fast IPIs, timers, performance counters)
     * 
     * Note that in the case of timers, we need to use the event register to determine which timer fired.
     */
    if (InterruptType == EXCEPT_AARCH64_FIQ) {
        if (AppleAicV2ReadIpiStatusRegister() & APPLE_FAST_IPI_STATUS_PENDING) {
            DEBUG((DEBUG_INFO, "Fast IPIs not supported yet, acking\n"));
            AppleAicV2WriteIpiStatusRegister(APPLE_FAST_IPI_STATUS_PENDING);
        }

        //Timers
        //Apple Silicon uses standard ARM timers, but just wired to FIQ. Hand off the FIQ as if it were a standard timer IRQ.
        if (ArmReadCntpCtl() & (ARM_ARCH_TIMER_ENABLE | ARM_ARCH_TIMER_ISTATUS))
        {
            DEBUG((DEBUG_INFO, "Physical timer FIQ asserted\n"));
            HwInterruptHandler = AicRegisteredInterruptHandlers[17];
            if(HwInterruptHandler != NULL)
            {
                //for now we hardcode the timer interrupt to 17.
                HwInterruptHandler(17, SystemContext);
            }
            else
            {
                DEBUG((DEBUG_ERROR, "Physical timer interrupt not assigned!\n"));
                ASSERT(FALSE);
            }

        }
        else if (ArmReadCntvCtl() & (ARM_ARCH_TIMER_ENABLE | ARM_ARCH_TIMER_ISTATUS))
        {
            DEBUG((DEBUG_INFO, "Virtual timer FIQ asserted\n"));
            HwInterruptHandler = AicRegisteredInterruptHandlers[17];
            if(HwInterruptHandler != NULL)
            {
                //ditto for the virtual timer.
                HwInterruptHandler(17, SystemContext);
            }
            else
            {
                DEBUG((DEBUG_ERROR, "Virtual timer interrupt not assigned!\n"));
                ASSERT(FALSE);
                return;
            }
        }

        //unknown: do we handle the el1 timers separately?

        //PMCs
        PmcStatus = AppleAicV2ReadPmcStatusRegister();
        UncorePmcStatus = AppleAicV2ReadUncorePmcControlRegister();
        if (PmcStatus & BIT11) {
            DEBUG((DEBUG_INFO, "PMCR0 FIQ asserted, unsupported, acking\n"));
            PmcStatus = PmcStatus & ~(BIT18 | BIT17 | BIT16);
            PmcStatus |= (BIT18 | BIT17 | BIT16 | BIT0);
            AppleAicV2WritePmcStatusRegister(PmcStatus);   
        }
        else if (FIELD_GET(APPLE_UPMCR0_IMODE, UncorePmcStatus) == APPLE_UPMCR_FIQ_IMODE && (AppleAicV2ReadUncorePmcStatusRegister() & APPLE_UPMSR_IACT))
        {
            DEBUG((DEBUG_INFO, "Uncore PMC FIQ asserted, unsupported, acking\n"));
            UncorePmcStatus = UncorePmcStatus & ~(APPLE_UPMCR0_IMODE);
            UncorePmcStatus |= APPLE_UPMCR_OFF_IMODE;
            AppleAicV2WriteUncorePmcControlRegister(UncorePmcStatus);
        }
    }


    /**
     * The IRQ case is much simpler, in all cases, read the event register, figure out what device
     * the IRQ originated from, jump to the IRQ handler assigned for that device.
     * 
     */
    else if (InterruptType == EXCEPT_AARCH64_IRQ) {
        
        if(HwInterruptHandler != NULL) {
            DEBUG((DEBUG_INFO, "Servicing AIC IRQ 0x%x\n", AicInterrupt));
            HwInterruptHandler(AicInterrupt, SystemContext);
        }
        else
        {
            DEBUG((DEBUG_ERROR, "Unassigned AIC IRQ: 0x%x\n", AicInterrupt));
            AppleAicV2EndOfInterrupt(&gHardwareInterruptAicV2Protocol, AicInterrupt);
        }

    }

    //SErrors for now are an instant panic.
    else if (InterruptType == EXCEPT_AARCH64_SERROR) {
        DEBUG((DEBUG_ERROR, "Unhandled SError!\n"));
        ASSERT(FALSE);
        return;
    }
}

// AIC protocol instance
EFI_HARDWARE_INTERRUPT_PROTOCOL  gHardwareInterruptV3Protocol = {
  RegisterInterruptSource,
  AppleAicV2UnmaskInterrupt,
  AppleAicV2MaskInterrupt,
  AppleAicV2GetInterruptState,
  AppleAicV2EndOfInterrupt
};


/**
 * Gets the type of trigger for a given IRQ.
 * 
 * @param This 
 * @param Source 
 * @param TriggerType 
 * @return STATIC 
 */
STATIC EFI_STATUS EFIAPI AppleAicV2GetIrqTriggerType(
    IN EFI_HARDWARE_INTERRUPT2_PROTOCOL *This,
    IN HARDWARE_INTERRUPT_SOURCE Source,
    OUT EFI_HARDWARE_INTERRUPT2_TRIGGER_TYPE *TriggerType
)
{
    /**
     * AIC does not have a facility to see if a given IRQ is level or edge triggered,
     * however, other than PCIe MSIs, every other IRQ is a level triggered interrupt.
     * 
     * As such, get the PCIe MSI IRQ number from the DT, compare it to Source, and if it matches,
     * state that this is a edge rising triggered interrupt. In all other cases, unconditionally indicate
     * a level triggered interrupt.
     */


}

/**
 * The ExitBootServices event. Will disable interrupts and shut down AIC hardware in handoff from DXE core to OS.
 * 
 * @param Event (input) - event being processed 
 * @param Context (input) - event context.
 * @return VOID 
 */
VOID EFIAPI AppleAicV2ExitBootServicesEvent(
    IN EFI_EVENT Event,
    IN VOID *Context
)
{
    UINTN InterruptIndex;

    //acknowledge any outstanding interrupts by reading the event register
    //(this will mask those interrupts at the same time)
    AppleAicAcknowledgeInterrupt();

    //mask all other interrupts
    for(InterruptIndex = 0; InterruptIndex < AicInfoStruct->NumIrqs; InterruptIndex++)
    {
        AppleAicV2MaskInterrupt(&gHardwareInterruptAicV2Protocol, InterruptIndex);
    }

    //disable the AIC controller
    MmioAnd32(AicV2Base + AIC_V2_CONFIG, ~(AIC_V2_CFG_ENABLE));

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
    UINT32 CoreID;
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

    /**
     * 
     * On Apple CPUs, IRQs can be opted out of by writing 
     * to a implementation defined MSR per-core. (S3_4_C15_C10_4)
     * 
     * Typically used when a core is in a state where it's undesirable for it to service IRQs.
     * (sleep for example)
     * 
     * For now we're only using 1 core (the others are in a spin loop, or 100% inactive) but when multi-core support is turned on,
     * if it turns out the hardware heuristic is sending IRQs to inactive cores, uncomment the below code to disable IRQs on
     * those other cores.
     * 
     */

    //CoreID = (ArmReadMpidr()) & (ARM_CORE_AFF0 | ARM_CORE_AFF1 | ARM_CORE_AFF2 | ARM_CORE_AFF3);
    // any result other than 0 implies that this is not core 0
    //if (CoreID != 0) {
    //    AppleArmDisableIrqsAndFiqs();
    //}

    //enable the AIC
    MmioOr32(AicV2Base + AIC_V2_CONFIG, AIC_V2_CFG_ENABLE);

    Status = InstallAndRegisterInterruptService(
        &gHardwareInterruptAicV2Protocol,
        &gHardwareInterrupt2AicV2Protocol,
        NULL,
        NULL
    );
    return Status;
}