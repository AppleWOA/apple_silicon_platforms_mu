/**
 * @file AppleAicLib.c
 * @author amarioguy (Arminder Singh)
 * @brief 
 * @version 1.9
 * @date 2022-09-24
 * 
 * @copyright Copyright (c) amarioguy (Arminder Singh), 2022.
 * 
 */
#include <Base.h>
#include <Library/ArmLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Include/libfdt.h>
#include <Library/AppleAicLib.h>


STATIC APPLE_AIC_VERSION mAicVersion;

/**
 * @brief Returns the version of AIC on the platform.
 * 
 * @return APPLE_AIC_VERSION_1 if using AICv1, else APPLE_AIC_VERSION_2 for AICv2.
 */
APPLE_AIC_VERSION EFIAPI AppleArmGetAicVersion(VOID)
{
    //sanity test: check that MIDR_EL1[31:24] has vendor code 0x61 (Apple) 
    UINT32 Midr = ArmReadMidr();
    if (Midr & 0x61000000)
    {
        //HACK - use a PCD containing the chip identifier for now
        //if we need to support platforms beyond desktop Apple silicon platforms, this behavior must change
        //(haven't figured out if there's an MSR or MMIO address that will give us this information)
        //default to AICv1 if the platform doesn't explicitly specify it's for AICv2
        if ((FixedPcdGet32(PcdAppleSocIdentifier) == 0x6000) || (FixedPcdGet32(PcdAppleSocIdentifier) == 0x8112))
        {
            mAicVersion = APPLE_AIC_VERSION_2;
            return APPLE_AIC_VERSION_2;
        }
        else
        {
            mAicVersion = APPLE_AIC_VERSION_1;
            return APPLE_AIC_VERSION_1;
        }

    }
    else
    {
        DEBUG((DEBUG_INFO, "Not on an Apple platform, AIC won't be present. Aborting\n"));
        return APPLE_AIC_VERSION_UNKNOWN;
    }
}


/**
 * Reads the event register on the platform AIC, which serves as an interrupt ack
 * (masks it at the same time)
 * 
 * TODO: use this in some meaningful way other than in the ExitBootServices event
 * 
 * @return UINT32 - Event register value
 */
UINT32 EFIAPI AppleAicAcknowledgeInterrupt(
    IN VOID
)
{
    return MmioRead32(AicInfoStruct->Regs.EventReg);
}

/**
 * Return the implemented number of IRQs on the platform.
 * 
 * @param AicBase - AIC base address
 * @return UINT32 - number of IRQs
 */
UINT32 EFIAPI AppleAicGetNumInterrupts(
    IN UINTN AicBase
)
{
    UINT32 NumIrqs;
    if(mAicVersion == APPLE_AIC_VERSION_2)
    {
        NumIrqs = MmioRead32(AicBase + AIC_V2_INFO_REG1) & AIC_V2_NUM_AND_MAX_IRQS_MASK;
    }
    else if (mAicVersion == APPLE_AIC_VERSION_1)
    {
        NumIrqs = MmioRead32(AicBase + AIC_V1_HW_INFO) & AIC_V2_NUM_AND_MAX_IRQS_MASK;
    }
    return NumIrqs;
}

/**
 * Returns the maximum supported number of IRQs on the SoC.
 * 
 * @param AicBase - AIC Base Address
 * @return UINT32 - max number of IRQs supported on the SoC.
 */
UINT32 EFIAPI AppleAicGetMaxInterrupts(
    IN UINTN AicBase
)
{
    UINT32 MaxIrqs;
    if(mAicVersion == APPLE_AIC_VERSION_2)
    {
        MaxIrqs = MmioRead32(AicBase + AIC_V2_INFO_REG3) & AIC_V2_NUM_AND_MAX_IRQS_MASK;
    }
    else if (mAicVersion == APPLE_AIC_VERSION_1)
    {
        //AICv1 is hard capped to 1024 IRQs
        MaxIrqs = 0x400;
    }
    return MaxIrqs;
}

/**
 * @brief Masks an IRQ by writing to the AIC's MASK_SET register.
 * 
 * Note that all FIQ sources are directly implemented in the cores instead of going through AIC.
 * 
 * TODO: deal with FIQs.
 * 
 * @param AicBase - AIC Base Address
 * @param Source - IRQ number 
 * 
 */
VOID EFIAPI AppleAicMaskInterrupt(
    IN UINTN AicBase,
    IN UINTN Source
)
{
    /**
     * Here's the general flow of how this works:
     * 
     * 1) Find out which CPU die the IRQ is on (if this is for AICv2)
     * 2) Calculate the bit of MASK_SET that needs to be written to.
     * 3) perform the memory write.
     * 
     * On AICv1, the CPU die calculation is skipped (there's only one CPU die on all AICv1 systems)
     * 
     * Note that the IRQ number must match the representation in hardware.
     * 
     **/
    UINT32 CpuDieNum = 0;
    DEBUG((DEBUG_INFO, "%a: masking interrupt 0x%llx\n", __FUNCTION__, Source));
    if (mAicVersion == APPLE_AIC_VERSION_2)
    {
        //the IRQ number will come from DeviceTree, which uses the interrupt numbers as they are in hardware
        CpuDieNum = (Source / AicInfoStruct->MaxIrqs) * AicInfoStruct->DieStride;
    }
    UINT32 IrqNum = Source % AicInfoStruct->MaxIrqs;

    MmioWrite32(AicBase + AicInfoStruct->Regs.IrqMaskSetRegOffset + CpuDieNum + AIC_MASK_REG(IrqNum), AIC_MASK_BIT(IrqNum));

}

/**
 * @brief Unmask an IRQ by writing to the AIC's MASK_SET register.
 * 
 * Note that all FIQ sources are directly implemented in the cores instead of going through AIC.
 * 
 * TODO: deal with FIQs.
 * 
 * @param AicBase - AIC Base Address
 * @param Source - IRQ number 
 * 
 */
VOID EFIAPI AppleAicUnmaskInterrupt(
    IN UINTN AicBase,
    IN UINTN Source
)
{
    /**
     * The general flow is the same as AppleAicMaskInterrupt, but instead of 
     * writing MASK_SET, we write MASK_CLR instead.
     **/
    UINT32 CpuDieNum = 0;
    DEBUG((DEBUG_INFO, "%a: unmasking interrupt 0x%llx\n", __FUNCTION__, Source));
    if (mAicVersion == APPLE_AIC_VERSION_2)
    {
        //the IRQ number will come from DeviceTree, which uses the interrupt numbers as they are in hardware
        CpuDieNum = Source / AicInfoStruct->MaxIrqs * AicInfoStruct->DieStride;
    }
    UINT32 IrqNum = Source % AicInfoStruct->MaxIrqs;

    MmioWrite32(AicBase + AicInfoStruct->Regs.IrqMaskSetRegOffset + CpuDieNum + AIC_MASK_REG(IrqNum), AIC_MASK_BIT(IrqNum));

}

/**
 * Read interrupt state from the AIC's HW_STATE register
 * 
 * @param AicBase - AIC base address
 * @param Source - IRQ number
 * @return TRUE if enabled, FALSE if disabled.
 */
BOOLEAN EFIAPI AppleAicReadInterruptState(
    IN UINTN AicBase,
    IN UINTN Source
)
{
    UINT32 CpuDieOffset = 0;
    DEBUG((DEBUG_INFO, "%a: reading interrupt state for IRQ number 0x%llx", __FUNCTION__, Source));
    if (mAicVersion == APPLE_AIC_VERSION_2)
    {
        CpuDieOffset = Source / AicInfoStruct->MaxIrqs * AicInfoStruct->DieStride;
    }
    UINT32 IrqNum = Source % AicInfoStruct->MaxIrqs;

    return MmioRead32(AicBase + AicInfoStruct->Regs.HwStateRegOffset + CpuDieOffset + AIC_MASK_REG(IrqNum)) & (AIC_MASK_BIT(IrqNum)) != 0;
}
