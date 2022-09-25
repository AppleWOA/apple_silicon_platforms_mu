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
#include <Library/AppleAicLib.h>


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
        //Check the SoC to see if we're using AICv1 or AICv2
    }
    else
    {
        DEBUG((DEBUG_INFO, "Not on an Apple platform, AIC won't be present. Aborting\n"));
        return RETURN_UNSUPPORTED;
    }
    return RETURN_SUCCESS;
}

