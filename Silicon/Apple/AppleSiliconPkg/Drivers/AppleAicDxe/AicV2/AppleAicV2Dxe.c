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
#include <Library/AppleSysRegs.h>
#include <Library/ArmGenericTimerCounterLib.h>
#include <Library/AppleDTLib.h>

#define APPLE_FAST_IPI_STATUS_PENDING BIT(0)

STATIC UINT64 AicV2Base;
AIC_INFO_STRUCT *AicInfoStruct;
STATIC UINT64 mAicV2IrqCfgOffset, mAicV2SoftwareSetRegOffset;
STATIC UINT64 mAicV2SoftwareClearRegOffset, mAicV2IrqMaskSetOffset;
STATIC UINT64 mAicV2IrqMaskClearOffset, mAicV2HwStateOffset;
STATIC UINT64 mAicV2EventReg;
STATIC APPLE_AIC_VERSION mAicVersion;

STATIC EFI_STATUS EFIAPI AppleAicV2CalculateRegisterOffsets(IN VOID);

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

    AppleAicMaskInterrupt(AicV2Base, Source, mAicV2IrqMaskSetOffset);
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

    AppleAicUnmaskInterrupt(AicV2Base, Source, mAicV2IrqMaskClearOffset);
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
    *State = AppleAicReadInterruptState(AicV2Base, Source, mAicV2HwStateOffset);
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
    //As of 11/4/2022 we are reserving IRQ numbers 17, 18, and 19 for timer FIQs.
    //If it's any one of these,disable the timer and return.
    if ((Source == 17) || (Source == 18) || (Source == 19))
    {
        ArmGenericTimerDisableTimer();
        return EFI_SUCCESS;
    }

    //reading the interrupt source in the event register acks and masks it at the same time
    //all we need to do is unmask it here. (note that for hardware IRQs, this assumes the hardware interrupt source has been cleared)
    AppleAicUnmaskInterrupt(AicV2Base, Source, mAicV2IrqMaskClearOffset);

    return EFI_SUCCESS;
}


/**
 * Calculate the AIC register offsets on the platform.
 * 
 * @return EFI_SUCCESS if successful, EFI_ERROR(-1) if an error occurred.
 */
STATIC EFI_STATUS EFIAPI AppleAicV2CalculateRegisterOffsets(IN VOID)
{
    dt_node_t *InterruptControllerNode = dt_get("aic");
    if (!InterruptControllerNode) {
        DEBUG((EFI_D_INFO | EFI_D_LOAD | EFI_D_ERROR, "no ADT supplied, exiting\n"));
        ASSERT(FALSE);
    }




    UINT64 StartOffset;
    UINT64 CurrentOffset;
    /**
     * Start with calculations that can be done with purely MMIO reads.
     * (AIC base is static, and the other values are easily derived through MMIO reads and bitwise ANDs.)
     * 
     */
    
    dt_node_reg(InterruptControllerNode, 0, &AicV2Base, NULL);
    AicInfoStruct->NumIrqs = AppleAicGetNumInterrupts(AicV2Base);
    AicInfoStruct->MaxIrqs = AppleAicGetMaxInterrupts(AicV2Base);

    if(mAicVersion == APPLE_AIC_VERSION_1){
        AicInfoStruct->MaxCpuDies = 1;
        AicInfoStruct->NumCpuDies = 1;
    }
    else if(mAicVersion == APPLE_AIC_VERSION_2){
        AicInfoStruct->MaxCpuDies = FIELD_GET(AIC_V2_INFO_REG3_MAX_DIE_COUNT_BITFIELD, MmioRead32(AicV2Base + AIC_V2_INFO_REG3));
        AicInfoStruct->NumCpuDies = (FIELD_GET(AIC_V2_INFO_REG1_LAST_CPU_DIE_BITFIELD, MmioRead32(AicV2Base + AIC_V2_INFO_REG1))) + 1;
    }

    
    /**
     * Calculate the offsets of registers that are dependent on the maximum number of IRQs supported.
     * 
     * The event register offset must be pulled from the devicetree, as it's the only place
     * where the value is stored, it cannot be read from MMIO.
     * 
     */

    /**
     * From IRQ_CFG + sizeof(UINT32) * MaxIrqs, the AIC registers are separated
     * by an offset of (sizeof(UINT32) * (MaxIrqs >> 5)).
     * 
     */
    StartOffset = mAicV2IrqCfgOffset = AIC_V2_IRQ_CFG_REG;

    if(mAicVersion == APPLE_AIC_VERSION_1){
        StartOffset = mAicV2IrqCfgOffset = AIC_TARGET_CPU;
        mAicV2EventReg = AicV2Base;
    }
    else if(mAicVersion == APPLE_AIC_VERSION_2){
        mAicV2EventReg = AicV2Base + dt_node_u32(InterruptControllerNode, "aic-iack-offset", 0);
    }

    CurrentOffset = StartOffset + sizeof(UINT32) * AicInfoStruct->MaxIrqs;

    //SW_SET
    mAicV2SoftwareSetRegOffset = CurrentOffset;
    CurrentOffset += sizeof(UINT32) * (AicInfoStruct->MaxIrqs >> 5);
    //SW_CLEAR
    mAicV2SoftwareClearRegOffset = CurrentOffset;
    CurrentOffset += sizeof(UINT32) * (AicInfoStruct->MaxIrqs >> 5);
    //MASK_SET
    mAicV2IrqMaskSetOffset = CurrentOffset;
    CurrentOffset += sizeof(UINT32) * (AicInfoStruct->MaxIrqs >> 5);
    //MASK_CLEAR
    mAicV2IrqMaskClearOffset = CurrentOffset;
    CurrentOffset += sizeof(UINT32) * (AicInfoStruct->MaxIrqs >> 5);
    CurrentOffset += sizeof(UINT32) * (AicInfoStruct->MaxIrqs >> 5);
    //HW_STATE (TODO: what is this reg meant for?)
    mAicV2HwStateOffset = CurrentOffset;
    AicInfoStruct->DieStride = CurrentOffset - StartOffset;
    AicInfoStruct->RegSize = (mAicV2EventReg - AicV2Base) + 4;
    
    return EFI_SUCCESS;

}

/**
 * EFI_CPU_INTERRUPT_HANDLER entered when a processor interrupt is taken.
 * 
 * Note that this handler handles both FIQs and IRQs. As SErrors are very bad at this stage of boot, let
 * the default exception handler deal with those and panic the system.
 * 
 * @param InterruptType - type of interrupt taken. will be EXCEPT_AARCH64_{IRQ, FIQ} for IRQs and FIQs respectively.
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
    HARDWARE_INTERRUPT_HANDLER TimerInterruptHandlerPhys;
    HARDWARE_INTERRUPT_HANDLER TimerInterruptHandlerVirt;
    UINT64 PmcStatus;
    UINT64 UncorePmcStatus;

    AicInterrupt = AppleAicAcknowledgeInterrupt(mAicV2EventReg);
    HwInterruptHandler = AicRegisteredInterruptHandlers[AicInterrupt];

    /**
     * In the FIQ case, every possible FIQ source must be checked to avoid an interrupt storm.
     * (Fast IPIs, timers, performance counters)
     * 
     * In the case of timers, both paths are checked, but in practice only one set of timers (phys or virt)
     * will be acted on due to how this firmware is configured.
     * 
     */
    if (InterruptType == EXCEPT_AARCH64_FIQ) {
        /**
         * 
         * Fast IPIs are not implemented yet, acknowledge but do not act upon them.
         * (Ideally, they won't be necessary at all given the nature of the firmware)
         * 
         */
        if (AppleAicV2ReadIpiStatusRegister() & APPLE_FAST_IPI_STATUS_PENDING) {
            DEBUG((DEBUG_INFO, "Fast IPIs not supported yet, acking\n"));
            AppleAicV2WriteIpiStatusRegister(APPLE_FAST_IPI_STATUS_PENDING);
        }

        /**
         * Timers
         * 
         * Apple Silicon uses standard ARM timers, but they are wired to FIQ. 
         * Hand off the FIQ as if it were a standard timer IRQ.
         * 
         * (Note that the interrupt number is software defined as timers do not have an associated hardware number)
         * 
         */
        if (
            ((ArmReadCntpCtl()) & 
            (ARM_ARCH_TIMER_ENABLE | ARM_ARCH_TIMER_IMASK | ARM_ARCH_TIMER_ISTATUS)) == (ARM_ARCH_TIMER_ENABLE | ARM_ARCH_TIMER_ISTATUS))
        {
            TimerInterruptHandlerPhys = AicRegisteredInterruptHandlers[17];
            //ArmGenericTimerDisableTimer();
            if(TimerInterruptHandlerPhys != NULL)
            {

                //for now we hardcode the timer interrupt to 17.
                TimerInterruptHandlerPhys(17, SystemContext);
            }
            else
            {
                //not having a timer interrupt assigned is very bad.
                DEBUG((DEBUG_ERROR, "Physical timer interrupt not assigned!\n"));
                ASSERT(FALSE);
            }

        }
        else if ((ArmReadCntvCtl() & (ARM_ARCH_TIMER_ENABLE | ARM_ARCH_TIMER_IMASK | ARM_ARCH_TIMER_ISTATUS)) == (ARM_ARCH_TIMER_ENABLE | ARM_ARCH_TIMER_ISTATUS))
        {
            TimerInterruptHandlerVirt = AicRegisteredInterruptHandlers[18];
            if(TimerInterruptHandlerVirt != NULL)
            {
                //ditto for the virtual timer.
                TimerInterruptHandlerVirt(18, SystemContext);
            }
            else
            {
                //not having a timer interrupt assigned is very bad.
                DEBUG((DEBUG_ERROR, "Virtual timer interrupt not assigned!\n"));
                ASSERT(FALSE);
            }
        }

        /**
         * 
         * PMC and Uncore PMC are both not implemented at this time, like Fast IPIs.
         * As with those, ack the interrupts if they come but don't act on them.
         * 
         */
        PmcStatus = AppleAicV2ReadPmcControlRegister();
        UncorePmcStatus = AppleAicV2ReadUncorePmcControlRegister();
        if (PmcStatus & BIT11) {
            DEBUG((DEBUG_INFO, "PMCR0 FIQ asserted, unsupported, acking\n"));
            PmcStatus = PmcStatus & ~(BIT18 | BIT17 | BIT16);
            PmcStatus |= (BIT18 | BIT17 | BIT16 | BIT0);
            AppleAicV2WritePmcControlRegister(PmcStatus);   
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
     * The software interrupt numbers have a one to one relationship with the hardware's understanding
     * of the interrupt numbers.
     * 
     */
    else if (InterruptType == EXCEPT_AARCH64_IRQ) {
        
        if(HwInterruptHandler != NULL) {
            HwInterruptHandler(AicInterrupt, SystemContext);
        }
        else
        {
            //if an interrupt is unassigned, ack it and exit.
            DEBUG((DEBUG_ERROR, "Unassigned AIC IRQ: 0x%x\n", AicInterrupt));
            AppleAicV2EndOfInterrupt(&gHardwareInterruptAicV2Protocol, AicInterrupt);
        }

    }
}

// AIC protocol instance
EFI_HARDWARE_INTERRUPT_PROTOCOL gHardwareInterruptAicV2Protocol = {
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
    dt_node_t *ApcieNode = dt_get("apcie");
    if (!ApcieNode) {
        DEBUG((EFI_D_ERROR, "no ADT supplied, exiting\n"));
        ASSERT(FALSE);
    }


    UINT32 EdgeTriggeredIrqNumStart = dt_node_u32(ApcieNode, "msi-vector-offset", 0);
    UINT32 EdgeTriggeredIrqNums = dt_node_u32(ApcieNode, "msi-vectors", 0);
    BOOLEAN IsEdgeTriggeredIrq = FALSE;
    /**
     * AIC does not have a facility to see if a given IRQ is level or edge triggered,
     * however, other than PCIe MSIs, every other IRQ is a level triggered interrupt.
     * 
     * As such, get the PCIe MSI IRQ number range from the DT, compare it to Source, and if it matches any of the IRQ numbers,
     * state that this is a edge rising triggered interrupt. In all other cases, unconditionally indicate
     * a level triggered interrupt.
     * 
     * TODO: some interrupts are level low triggered (namely I2C interrupts), account for these.
     */

    DEBUG((DEBUG_VERBOSE, "Edge triggered IRQ number start: %d", EdgeTriggeredIrqNumStart));
    for(UINT64 IrqNum = EdgeTriggeredIrqNumStart; IrqNum < (EdgeTriggeredIrqNumStart + EdgeTriggeredIrqNums); IrqNum++)
    {
        if(Source == IrqNum)
        {
            DEBUG((DEBUG_VERBOSE, "%d is an edge triggered IRQ\n", (UINT32)Source));
            IsEdgeTriggeredIrq = TRUE;
            break;
        }
    }
    if(IsEdgeTriggeredIrq)
    {
        *TriggerType = EFI_HARDWARE_INTERRUPT2_TRIGGER_EDGE_RISING;
    }
    else {
        *TriggerType = EFI_HARDWARE_INTERRUPT2_TRIGGER_LEVEL_HIGH;
    }
    return EFI_SUCCESS;

}

STATIC EFI_STATUS EFIAPI AppleAicV2SetIrqTriggerType(
    IN EFI_HARDWARE_INTERRUPT2_PROTOCOL *This,
    IN HARDWARE_INTERRUPT_SOURCE Source,
    IN EFI_HARDWARE_INTERRUPT2_TRIGGER_TYPE TriggerType
)
{
    //unsupported for now, just return EFI_SUCCESS
    return EFI_SUCCESS;
}

// AIC HardwareInterrupt2 protocol instance
EFI_HARDWARE_INTERRUPT2_PROTOCOL gHardwareInterrupt2AicV2Protocol = {
  (HARDWARE_INTERRUPT2_REGISTER)RegisterInterruptSource,
  (HARDWARE_INTERRUPT2_ENABLE)AppleAicV2UnmaskInterrupt,
  (HARDWARE_INTERRUPT2_DISABLE)AppleAicV2MaskInterrupt,
  (HARDWARE_INTERRUPT2_INTERRUPT_STATE)AppleAicV2GetInterruptState,
  (HARDWARE_INTERRUPT2_END_OF_INTERRUPT)AppleAicV2EndOfInterrupt,
  AppleAicV2GetIrqTriggerType,
  AppleAicV2SetIrqTriggerType
};



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
    UINT32 AicConfigValue = (UINT32)(AIC_V2_CFG_ENABLE);
    AicConfigValue = ~AicConfigValue;

    //acknowledge any outstanding interrupts by reading the event register.
    //(this will mask those interrupts at the same time)
    AppleAicAcknowledgeInterrupt(mAicV2EventReg);

    //mask all other interrupts by writing to MASK_SET
    for(InterruptIndex = 0; InterruptIndex < AicInfoStruct->NumIrqs; InterruptIndex++)
    {
        AppleAicV2MaskInterrupt(&gHardwareInterruptAicV2Protocol, InterruptIndex);
    }

    //disable the AIC controller
    MmioAnd32(AicV2Base + AIC_V2_CONFIG, AicConfigValue);

}

/**
 * Prepares the AIC protocol for use by the DXE environment.
 * 
 * @param ImageHandle
 * @param SystemTable 
 * @return EFI_STATUS 
 */

EFI_STATUS AppleAicV2DxeInit(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable, APPLE_AIC_VERSION aicVersion)
{

    mAicVersion = aicVersion;

    UINTN InterruptIndex;
    EFI_STATUS Status;
    UINT32 AicV2NumInterrupts;
    UINT32 AicV2MaxInterrupts;

    //DEBUG((DEBUG_INFO, "%a: AIC driver start\n", __FUNCTION__));
    AicInfoStruct = AllocatePool(sizeof(AIC_INFO_STRUCT));
    //multi-core support not added yet
    //UINT32 CoreID;
    //Assert that the Hardware Interrupt protocol is not installed already.
    ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gHardwareInterruptProtocolGuid);

    //set up and collect variables in global variables that will get passed to functions that need them.
    Status = AppleAicV2CalculateRegisterOffsets();
    AicV2NumInterrupts = AicInfoStruct->NumIrqs;
    AicV2MaxInterrupts = AicInfoStruct->MaxIrqs;
    
    DEBUG((DEBUG_INFO, "AICv2 with %u/%u configured IRQs, at 0x%llx\n", AicV2NumInterrupts, AicV2MaxInterrupts, AicV2Base));
    DEBUG((DEBUG_INFO, "%u/%u CPU dies, Die Stride: 0x%lx\n", AicInfoStruct->NumCpuDies, AicInfoStruct->MaxCpuDies, AicInfoStruct->DieStride));
    DEBUG((DEBUG_VERBOSE, "AIC register addresses: \n"
    "\tAIC_V2_BASE: 0x%llx\n"
    "\tAIC_V2_IRQ_CFG: 0x%llx\n"
    "\tAIC_V2_SW_SET: 0x%llx\n"
    "\tAIC_V2_SW_CLR: 0x%llx\n"
    "\tAIC_V2_MASK_SET: 0x%llx\n"
    "\tAIC_V2_MASK_CLR: 0x%llx\n"
    "\tAIC_V2_HW_STATE: 0x%llx\n"
    "\tAIC_V2_EVENT: 0x%llx\n\n",
    AicV2Base,
    (AicV2Base + mAicV2IrqCfgOffset),
    (AicV2Base + mAicV2SoftwareSetRegOffset),
    (AicV2Base + mAicV2SoftwareClearRegOffset),
    (AicV2Base + mAicV2IrqMaskSetOffset),
    (AicV2Base + mAicV2IrqMaskClearOffset),
    (AicV2Base + mAicV2HwStateOffset),
    (mAicV2EventReg)
    ));

    //enable the AIC
    DEBUG((DEBUG_VERBOSE, "%a: enabling AIC\n", __FUNCTION__));
    MmioOr32(AicV2Base + AIC_V2_CONFIG, AIC_V2_CFG_ENABLE);

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

    //register the interrupt controller now that setup is done.

    Status = InstallAndRegisterInterruptService(
        &gHardwareInterruptAicV2Protocol,
        &gHardwareInterrupt2AicV2Protocol,
        AppleAicV2InterruptHandler,
        AppleAicV2ExitBootServicesEvent
    );
    return Status;
}