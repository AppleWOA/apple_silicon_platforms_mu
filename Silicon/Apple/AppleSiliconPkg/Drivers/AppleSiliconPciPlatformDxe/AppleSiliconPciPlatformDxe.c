/**
 * Copyright (c) 2023, amarioguy (AppleWOA authors).
 * 
 * Module Name:
 *     AppleSiliconPciPlatformDxe.c
 * 
 * Abstract:
 *     Platform specific driver for Apple silicon platforms to bring up PCIe root ports/link devices.
 * 
 * Environment:
 *     UEFI DXE (Driver Execution Environment).
 * 
 * License:
 *     SPDX-License-Identifier: BSD-2-Clause-Patent OR MIT
 * 
*/

#include <PiDxe.h>
#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/ArmLib.h>
#include <Library/PrintLib.h>
#include <Library/PcdLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/PciLib.h>
#include <Library/PciSegmentLib.h>
#include <Library/PciExpressLib.h>
#include <IndustryStandard/Pci.h>
#include <Library/TimerLib.h>

//
// This driver's purpose is to bring up the PCIe root ports and link devices.
// As such, this also needs to register MSIs (which will be handled by the AIC driver)
// This will be run after AIC is brought up, but before the default UEFI PCIe driver runs.
//

EFI_STATUS
EFIAPI 
AppleSiliconPciPlatformDxeInitialize(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
) 
{
    //
    // The PCIe controller itself seems to be brought up by m1n1 itself, including tunables,
    // so we just need to bring up root ports here so that DXE can detect the devices on the ports, including the XHCI controller.
    //
}