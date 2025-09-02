/**
 * Copyright (c) 2025, AppleWOA authors.
 * 
 * Module Name:
 *     AppleAicV1Dxe.c
 * 
 * Abstract:
 *     AICv1 interrupt controller initialization and driver code for the UEFI environment.
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

#define APPLE_FAST_IPI_STATUS_PENDING BIT(0)

//
// AICv1 has a lot of the same semantic operations as AICv2, having separate set/clear registers
// for interrupts, the critical difference is really in how interrupt routing is handled,
// with AICv1 using traditional interrupt affinity routing, along with having support for "slow" IPIs via
// the AIC itself (making it closer to a traditional GIC in that sense.)
// 
// The plan for this firmware is to rely on AIC-based IPIs over Fast IPIs on AICv1 platforms.
// (less FIQs to handle is always a benefit in the Windows case...)
//

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

//
// Description:
//  Masks an interrupt on the AICv1. Note that this does not include the timer interrupt on platforms
//  that only use FIQs for the timer.
//
// Return status:
//  EFI_SUCCESS, failures will ASSERT() the platform.
//
STATIC EFI_STATUS EFIAPI AppleAicV1MaskInterrupt(
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

    AppleAicMaskInterrupt(AicBase, Source, mAicMaskSetRegOffset);
    return EFI_SUCCESS;
}

//
// Description:
//  Unmasks an interrupt on the AICv1. Note that this does not include the timer interrupt on platforms
//  that only use FIQs for the timer.
//
// Return status:
//  EFI_SUCCESS, failures will ASSERT() the platform.
//
STATIC EFI_STATUS EFIAPI AppleAicV1UnmaskInterrupt(
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

    AppleAicUnmaskInterrupt(AicBase, Source, mAicMaskClearRegOffset);
    return EFI_SUCCESS;
}

//
// Description:
//  Gets the state of an interrupt on the AICv1.
//
// Return status:
//  EFI_SUCCESS, failures will ASSERT() the platform.
//
STATIC EFI_STATUS EFIAPI AppleAicV1GetInterruptState(
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
    *State = AppleAicReadInterruptState(AicBase, Source, mAicMaskSetRegOffset);
    return EFI_SUCCESS;
}

//
// Description:
//  Signals the EOI (end-of-interrupt) signal for an interrupt on the AICv1.
//
// Return status:
//  EFI_SUCCESS
//
STATIC EFI_STATUS EFIAPI AppleAicV1EndOfInterrupt(
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
    AppleAicUnmaskInterrupt(AicBase, Source, mAicMaskClearRegOffset);

    return EFI_SUCCESS;
}

// AIC protocol instance
EFI_HARDWARE_INTERRUPT_PROTOCOL gHardwareInterruptAicV1Protocol = {
  RegisterInterruptSource,
  AppleAicV1UnmaskInterrupt,
  AppleAicV1MaskInterrupt,
  AppleAicV1GetInterruptState,
  AppleAicV1EndOfInterrupt
};

//
// Description:
//  The ExitBootServices event, disables and masks interrupts before handoff to the OS.
//
// Return value:
//  None.
VOID EFIAPI AppleAicV1ExitBootServicesEvent(
    IN EFI_EVENT Event,
    IN VOID *Context
)
{
    UINTN InterruptIndex;

    //acknowledge any outstanding interrupts by reading the event register.
    //(this will mask those interrupts at the same time)
    AppleAicAcknowledgeInterrupt((AicBase + mAicEventRegOffset));

    //mask all other interrupts by writing to MASK_SET
    for(InterruptIndex = 0; InterruptIndex < AicInfoStruct->NumIrqs; InterruptIndex++)
    {
        AppleAicV1MaskInterrupt(&gHardwareInterruptAicV1Protocol, InterruptIndex);
    }

    //disable the AIC controller (not needed on AICv1?)

}

/**
 * Gets the type of trigger for a given IRQ.
 * 
 * @param This 
 * @param Source 
 * @param TriggerType 
 * @return STATIC 
 */
STATIC EFI_STATUS EFIAPI AppleAicV1GetIrqTriggerType(
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

STATIC EFI_STATUS EFIAPI AppleAicV1SetIrqTriggerType(
    IN EFI_HARDWARE_INTERRUPT2_PROTOCOL *This,
    IN HARDWARE_INTERRUPT_SOURCE Source,
    IN EFI_HARDWARE_INTERRUPT2_TRIGGER_TYPE TriggerType
)
{
    //unsupported for now, just return EFI_SUCCESS
    return EFI_SUCCESS;
}

// AIC HardwareInterrupt2 protocol instance
EFI_HARDWARE_INTERRUPT2_PROTOCOL gHardwareInterrupt2AicV1Protocol = {
  (HARDWARE_INTERRUPT2_REGISTER)RegisterInterruptSource,
  (HARDWARE_INTERRUPT2_ENABLE)AppleAicV1UnmaskInterrupt,
  (HARDWARE_INTERRUPT2_DISABLE)AppleAicV1MaskInterrupt,
  (HARDWARE_INTERRUPT2_INTERRUPT_STATE)AppleAicV1GetInterruptState,
  (HARDWARE_INTERRUPT2_END_OF_INTERRUPT)AppleAicV1EndOfInterrupt,
  AppleAicV1GetIrqTriggerType,
  AppleAicV1SetIrqTriggerType
};


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
STATIC VOID EFIAPI AppleAicV1InterruptHandler(
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

    AicInterrupt = AppleAicAcknowledgeInterrupt((AicBase + mAicEventRegOffset));
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
        if (AppleAicV1ReadIpiStatusRegister() & APPLE_FAST_IPI_STATUS_PENDING) {
            DEBUG((DEBUG_INFO, "Fast IPIs not supported yet, acking\n"));
            AppleAicV1WriteIpiStatusRegister(APPLE_FAST_IPI_STATUS_PENDING);
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
        PmcStatus = AppleAicV1ReadPmcControlRegister();
        UncorePmcStatus = AppleAicV1ReadUncorePmcControlRegister();
        if (PmcStatus & BIT11) {
            DEBUG((DEBUG_INFO, "PMCR0 FIQ asserted, unsupported, acking\n"));
            PmcStatus = PmcStatus & ~(BIT18 | BIT17 | BIT16);
            PmcStatus |= (BIT18 | BIT17 | BIT16 | BIT0);
            AppleAicV1WritePmcControlRegister(PmcStatus);   
        }
        else if (FIELD_GET(APPLE_UPMCR0_IMODE, UncorePmcStatus) == APPLE_UPMCR_FIQ_IMODE && (AppleAicV2ReadUncorePmcStatusRegister() & APPLE_UPMSR_IACT))
        {
            DEBUG((DEBUG_INFO, "Uncore PMC FIQ asserted, unsupported, acking\n"));
            UncorePmcStatus = UncorePmcStatus & ~(APPLE_UPMCR0_IMODE);
            UncorePmcStatus |= APPLE_UPMCR_OFF_IMODE;
            AppleAicV1WriteUncorePmcControlRegister(UncorePmcStatus);
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
            AppleAicV1EndOfInterrupt(&gHardwareInterruptAicV1Protocol, AicInterrupt);
        }

    }
}

//
// Description:
//  Initializes the AICv1 device. Failures usually assert the system outright.
//
// Return status:
//  EFI_SUCCESS if the HARDWARE_INTERRUPT_CONTROLLER and HARDWARE_INTERRUPT_CONTROLLER2 protocols
//  install successfully, 0 otherwise.
//
EFI_STATUS AppleAicV1DxeInit(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) 
{
    UINTN InterruptIndex;
    EFI_STATUS Status;
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
        DEBUG((EFI_D_INFO | EFI_D_LOAD | EFI_D_ERROR, "no ADT supplied!\n"));
        ASSERT(FALSE);
    }
    dt_node_reg(InterruptControllerNode, 0, &AicBase, NULL);

    AicInfoStruct->NumIrqs = AppleAicGetNumInterrupts(AicBase);
    AicInfoStruct->MaxIrqs = AppleAicGetMaxInterrupts(AicBase);

    AicInfoStruct->MaxCpuDies = 1;
    AicInfoStruct->NumCpuDies = 1;

    AicInfoStruct->DieStride = 0; // no die stride for AICv1.

    mAicEventRegOffset = 0; //per Linux AIC driver this is correct?
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

    Status = InstallAndRegisterInterruptService(
        &gHardwareInterruptAicV1Protocol,
        &gHardwareInterrupt2AicV1Protocol,
        AppleAicV1InterruptHandler,
        AppleAicV1ExitBootServicesEvent
    );
    return EFI_SUCCESS;
}
