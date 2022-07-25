/**
 * @file AppleArmPsciHvcResetSystemLib.c
   Support PSCI reset system functionality on Apple platforms

   Note: based on ArmPkg/Library/ArmPsciResetSystemLib

   The primary difference in this PSCI library is that on Apple platforms,
   EL3 is unimplemented, so we need to use m1n1/the hypervisor layer
   to implement PSCI methods like system reset

   If we are running in EL2, we will need to instead issue a "genter" instruction to enter GL2 (guarded EL2, Apple's idea of secure world)
   and implement PSCI methods there.

   Note that this will assume that SPRR/GXF were set up before entry into UEFI and behavior of genter without this set up is undefined.

   Other than the instruction used to call into EL2 or GL2, this library remains functionally identical otherwise
 
   Copyright (c) 2022, amarioguy (Arminder Singh). 

   SPDX-License-Identifier: BSD-2-Clause-Patent
 **/

#include <PiDxe.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/EfiResetSystemLib.h>
#include <Library/ArmHvcLib.h>
//#include <Library/AppleArmGxfHvcLib.h>

#include <IndustryStandard/ArmStdSmc.h>

EFI_STATUS EFIAPI LibResetSystem(IN EFI_RESET_TYPE SysResetType, OUT EFI_STATUS ResetStatus, IN UINTN DataSize, IN VOID *ResetData OPTIONAL)
{

  //TODO: implement GXF support
  ARM_HVC_ARGS PsciHvcArgs;

  switch (SysResetType)
  {
    
    // fall through to cold reboot for warm and platform specific reboots (warm can eventually be implemented)
    case EfiResetWarm:
    case EfiResetPlatformSpecific:
    case EfiResetCold:
      PsciHvcArgs.Arg0 = ARM_SMC_ID_PSCI_SYSTEM_RESET;
      break;
    case EfiResetShutdown:
      PsciHvcArgs.Arg0 = ARM_SMC_ID_PSCI_SYSTEM_OFF;
      break;
    default:
      // the requested PSCI system reset functionality is not implemented
      DEBUG ((DEBUG_ERROR, "AppleArmPsciResetSystemLib: PSCI Method not implemented\n"));
    
    //Make the hypercall to perform the PSCI action requested in arg0
    //TODO: implement a selector mechanism based on current EL, just use standard Hvc for now

    ArmCallHvc(&PsciHvcArgs);

    //if we come here, something has gone *very* wrong
    DEBUG ((DEBUG_ERROR, "AppleArmPsciResetSystemLib: PSCI method failed, looping in place...\n"));
    CpuDeadLoop();
  }
}