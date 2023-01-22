/**
 * @file AppleArmPsciHvcResetSystemLib.c
   Support PSCI reset system functionality on Apple platforms

   Note: based on ArmPkg/Library/ArmSmcPsciResetSystemLib

   The primary difference in this PSCI library is that on Apple platforms,
   EL3 is unimplemented, so if possible it's best to stick to HVC calls to handle PSCI

   NOTE: if EL2 can handle SMC exceptions, then this entire library will no longer be necessary and in that case this implementation will use the standard ArmSmcPsciResetSystemLib

   If we are running in EL2, we will instead issue a "genter" instruction to enter GL2 (guarded EL2, Apple's idea of secure world)
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

//TODO: implement GXF support

/**
  This function causes a system-wide reset (cold reset), in which
  all circuitry within the system returns to its initial state. This type of reset
  is asynchronous to system operation and operates without regard to
  cycle boundaries.

  If this function returns, it means that the system does not support cold reset.
**/
VOID
EFIAPI
ResetCold (
  VOID
  )
{
  ARM_HVC_ARGS ColdResetArgs;
  ColdResetArgs.Arg0 = ARM_SMC_ID_PSCI_SYSTEM_RESET;
  ColdResetArgs.Arg1 = 0;
  ColdResetArgs.Arg2 = 0;
  ColdResetArgs.Arg3 = 0;
  // Send a PSCI 0.2 SYSTEM_RESET command
  ArmCallHvc(&ColdResetArgs);
}

/**
  This function causes a system-wide initialization (warm reset), in which all processors
  are set to their initial state. Pending cycles are not corrupted.

  If this function returns, it means that the system does not support warm reset.
**/
VOID
EFIAPI
ResetWarm (
  VOID
  )
{
  // Map a warm reset into a cold reset
  ResetCold ();
}

/**
  This function causes the system to enter a power state equivalent
  to the ACPI G2/S5 or G3 states.

  If this function returns, it means that the system does not support shutdown reset.
**/
VOID
EFIAPI
ResetShutdown (
  VOID
  )
{
  ARM_HVC_ARGS ShutdownArgs;
  ShutdownArgs.Arg0 = ARM_SMC_ID_PSCI_SYSTEM_RESET;
  ShutdownArgs.Arg1 = 0;
  ShutdownArgs.Arg2 = 0;
  ShutdownArgs.Arg3 = 0;
  // Send a PSCI 0.2 SYSTEM_OFF command
  ArmCallHvc(&ShutdownArgs);
}

/**
  This function causes a systemwide reset. The exact type of the reset is
  defined by the EFI_GUID that follows the Null-terminated Unicode string passed
  into ResetData. If the platform does not recognize the EFI_GUID in ResetData
  the platform must pick a supported reset type to perform.The platform may
  optionally log the parameters from any non-normal reset that occurs.

  @param[in]  DataSize   The size, in bytes, of ResetData.
  @param[in]  ResetData  The data buffer starts with a Null-terminated string,
                         followed by the EFI_GUID.
**/
VOID
EFIAPI
ResetPlatformSpecific (
  IN UINTN  DataSize,
  IN VOID   *ResetData
  )
{
  // Map the platform specific reset as reboot
  ResetCold ();
}


/**
  The ResetSystem function resets the entire platform.

  @param[in] ResetType      The type of reset to perform.
  @param[in] ResetStatus    The status code for the reset.
  @param[in] DataSize       The size, in bytes, of ResetData.
  @param[in] ResetData      For a ResetType of EfiResetCold, EfiResetWarm, or EfiResetShutdown
                            the data buffer starts with a Null-terminated string, optionally
                            followed by additional binary data. The string is a description
                            that the caller may use to further indicate the reason for the
                            system reset.
**/
VOID
EFIAPI
ResetSystem (
  IN EFI_RESET_TYPE  ResetType,
  IN EFI_STATUS      ResetStatus,
  IN UINTN           DataSize,
  IN VOID            *ResetData OPTIONAL
  )
{
  switch (ResetType) {
    case EfiResetWarm:
      ResetWarm ();
      break;

    case EfiResetCold:
      ResetCold ();
      break;

    case EfiResetShutdown:
      ResetShutdown ();
      return;

    case EfiResetPlatformSpecific:
      ResetPlatformSpecific (DataSize, ResetData);
      return;

    default:
      return;
  }
}
