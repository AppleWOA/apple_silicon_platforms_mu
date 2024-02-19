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
#include <Include/libfdt.h>
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

STATIC EFI_STATUS AppleSiliconPciePlatformDxeGetResetGpios(INT32 SubNode, INT32 Index, APPLE_PCIE_GPIO_DESC *Desc) {

}

STATIC EFI_STATUS AppleSiliconPciePlatformDxeSetupPciePort(APPLE_PCIE_COMPLEX_INFO *PcieComplex, INT32 SubNode) {
  APPLE_PCIE_DEVICE_PORT_INFO *PciePortInfo = AllocateZeroPool(sizeof(APPLE_PCIE_DEVICE_PORT_INFO));
  EFI_STATUS Status;
  UINT32 *IndexPtr;
  UINT32 Index;
  UINT32 IndexLength;
  APPLE_PCIE_GPIO_DESC ResetGpioStruct;
  UINT64 FdtBlob = PcdGet64(PcdFdtPointer);
  CHAR8 PortName[10];
  
  //
  // GPIO setup
  // Using the Asahi Linux U-Boot method for now, might want to switch to UEFI specific
  // methods later, but I need to get this running now.
  //
  Status = AppleSiliconPciePlatformDxeGetResetGpios(SubNode, 0, &ResetGpioStruct);

  IndexPtr = fdt_getprop((VOID *)FdtBlob, SubNode, "reg", &IndexLength);
  Index = IndexPtr[0];

  PciePortInfo->Complex = PcieComplex;
  PciePortInfo->DevicePortIndex = Index >> 11;
  PciePortInfo->PortSubNode = SubNode;

  //
  // From the root PCIe0 node, get the addresses of each of the ports.
  //
  AsciiSPrint(PortName, sizeof(PortName), "port%d", PciePortInfo->DevicePortIndex);

  //
  // TODO: the rest of this, as of 1-11-2024 going back to strictly RAMDisk based testing.
  //
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI 
AppleSiliconPciPlatformDxeInitialize(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
) 
{
    UINT64 FdtBlob = PcdGet64(PcdFdtPointer);
    INT32 Length;
    INT32 PcieNode = fdt_path_offset((VOID *)FdtBlob, "/soc/pcie");
    CONST INT32 *PcieRegs = fdt_getprop((VOID *)FdtBlob, PcieNode, "reg", &Length);
    APPLE_PCIE_COMPLEX_INFO *PcieComplexInfoStruct = AllocateZeroPool(sizeof(APPLE_PCIE_COMPLEX_INFO));
    CHAR8 *PortStatus;
    INT32 PortStatusLength;
    INT32 PcieSubNode;
    EFI_STATUS Status;

    //
    // The PCIe controller itself seems to be brought up by m1n1 itself, including tunables,
    // so we just need to bring up root ports here so that DXE can detect the devices on the ports, including the XHCI controller.
    //

    //
    // Pull the ECAM base address from the FDT. Ditto for "rc" base address.
    // With the Asahi Linux FDTs, we can usually safely assume this to be the first reg entry,
    // but if this ever changes (as will be the case when booting right from iBoot using ADT), this approach will need to change too.
    // TODO: see how to do this with ADT on embedded or non-m1n1 case.
    //
    PcieComplexInfoStruct->EcamCfgRegionBase = fdt32_to_cpu(PcieRegs[0]);
    PcieComplexInfoStruct->EcamCfgRegionBase = ((PcieComplexInfoStruct->EcamCfgRegionBase) << 32) | (fdt32_to_cpu(PcieRegs[1]));
    PcieComplexInfoStruct->EcamCfgRegionSize = fdt32_to_cpu(PcieRegs[3]);
    PcieComplexInfoStruct->RcRegionBase = fdt32_to_cpu(PcieRegs[4]);
    PcieComplexInfoStruct->RcRegionBase = ((PcieComplexInfoStruct->RcRegionBase) << 32) | (fdt32_to_cpu(PcieRegs[5]));

    //
    // NOTE: the following does NOT account for APCIe-GE devices such as the Mac Pros.
    //
    fdt_for_each_subnode(PcieSubNode, (VOID *)FdtBlob, PcieNode) {
      PortStatus = fdt_getprop((VOID *)FdtBlob, PcieSubNode, "status", &PortStatusLength);
      if(!strcmp(PortStatus, "disabled")) {
        DEBUG((DEBUG_INFO, "PCIe port disabled, continuing\n"));
        continue;
      }
      Status = AppleSiliconPciePlatformDxeSetupPciePort(PcieComplexInfoStruct, PcieSubNode);
      if(EFI_ERROR(Status)) {
        DEBUG((DEBUG_ERROR, "%a - Port setup failed: %r\n", __FUNCTION__, Status));
        ASSERT_EFI_ERROR(Status);
      }
    }

    return EFI_SUCCESS;
}