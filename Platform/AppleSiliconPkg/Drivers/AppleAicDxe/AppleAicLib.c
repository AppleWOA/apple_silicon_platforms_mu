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
        mAicVersion = APPLE_AIC_VERSION_1;
        return APPLE_AIC_VERSION_1;
    }
    else
    {
        DEBUG((DEBUG_INFO, "Not on an Apple platform, AIC won't be present. Aborting\n"));
        return APPLE_AIC_VERSION_UNKNOWN;
    }
}

UINTN AppleAicGetMaxInterrupts(VOID)
{
    UINTN NumIrqs;
    if(mAicVersion == APPLE_AIC_VERSION_2)
    {
        NumIrqs = MmioRead32((FixedPcdGet64(PcdAicInterruptControllerBase)) + AIC_V2_INFO_REG1) & AIC_V2_NUM_IRQS_MASK;
    }
    else if (mAicVersion == APPLE_AIC_VERSION_1)
    {
        NumIrqs = MmioRead32((FixedPcdGet64(PcdAicInterruptControllerBase)) + 0x0004) & AIC_V2_NUM_IRQS_MASK;
    }
}

