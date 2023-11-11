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
 *     SPDX-License-Identifier: (BSD-2-Clause-Patent OR MIT) AND GPL-2.0
 * 
 *     Original code basis is from the Asahi Linux project, original copyright and author notices below.
 *     Copyright (C) 2021 Alyssa Rosenzweig <alyssa@rosenzweig.io>
 *     Copyright (C) 2021 Google LLC
 *     Copyright (C) 2021 Corellium LLC
 *     Copyright (C) 2021 Mark Kettenis <kettenis@openbsd.org>
 * 
 *     Original authors of the Linux code:
 *     Author: Alyssa Rosenzweig <alyssa@rosenzweig.io>
 *     Author: Marc Zyngier <maz@kernel.org>
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
#include <Drivers/AppleSiliconPciPlatformDxe.h>

//
// This driver's purpose is to bring up the PCIe root ports and link devices.
// As such, this also needs to register MSIs (which will be handled by the AIC driver)
// This will be run after AIC is brought up, but before the default UEFI PCIe driver runs.
//

//
// TODO: check if this is required on embedded, very real chance iBoot does the bringup itself,
// especially on later versions or later hardware.
//

typedef struct ApplePcieComplexInfo {
  UINT64 EcamCfgRegionBase, // base address for ECAM region, pull from DT
};

typedef struct ApplePcieDevicePortInfo {
  UINT64 DeviceBaseAddress // base address for the device port in the PCIe complex, pull from DT
}

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
    for(UINT32 i = 0)
}