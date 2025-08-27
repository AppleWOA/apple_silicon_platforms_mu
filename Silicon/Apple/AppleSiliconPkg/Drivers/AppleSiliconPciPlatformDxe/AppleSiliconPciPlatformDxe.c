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
#include <Protocol/EmbeddedGpio.h>
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


//
// Helpers to do MMIO reads and writes
//


//
// Description:
//   Gets the reset GPIO number for a given PCIe port. ASSERT's on failure.
//
// Return values:
//   EFI_SUCCESS - GPIO number found

STATIC EFI_STATUS AppleSiliconPciePlatformDxeGetResetGpios(dt_node_t *SubNode, INT32 Index, APPLE_PCIE_GPIO_DESC *Desc) {
  UINTN len;
  //
  UINT32 *FunctionPerst = (UINT32 *)dt_node_prop(SubNode, "function-perst", &len);
  //
  // Get the reset GPIO values from the node.
  //

  Desc->GpioNum = FunctionPerst[2];
  Desc->GpioActivePolarity = FunctionPerst[3] == 0;//TODO: properly handle this 
  DEBUG((DEBUG_INFO, "%a - GPIO found is 0x%x, polarity is 0x%x\n", __FUNCTION__, Desc->GpioNum, Desc->GpioActivePolarity));
  return EFI_SUCCESS;

}

STATIC VOID AppleSiliconPcieSetBits(UINT32 BitsToSet, UINTN Address) {
  UINT32 OriginalValue = MmioRead32(Address);
  DEBUG((DEBUG_INFO, "%a - Original MMIO value for 0x%llx is 0x%x\n", __FUNCTION__, Address, OriginalValue));
  UINT32 UpdatedValue = OriginalValue | BitsToSet;
  DEBUG((DEBUG_INFO, "%a - MMIO value to be written to 0x%llx is 0x%x\n", __FUNCTION__, Address, UpdatedValue));
  MmioWrite32(Address, UpdatedValue);
  UINT32 NewValue = MmioRead32(Address);
  DEBUG((DEBUG_INFO, "%a - MMIO value for 0x%llx that was read back is 0x%x\n", __FUNCTION__, Address, NewValue));

}

STATIC VOID AppleSiliconPcieClearBits(UINT32 BitsToClear, UINTN Address) {
  UINT32 OriginalValue = MmioRead32(Address);
  DEBUG((DEBUG_INFO, "%a - Original MMIO value for 0x%llx is 0x%x\n", __FUNCTION__, Address, OriginalValue));
  UINT32 UpdatedValue = OriginalValue & ((~BitsToClear));
  DEBUG((DEBUG_INFO, "%a - MMIO value to be written to 0x%llx is 0x%x\n", __FUNCTION__, Address, UpdatedValue));
  MmioWrite32(Address, UpdatedValue);
  UINT32 NewValue = MmioRead32(Address);
  DEBUG((DEBUG_INFO, "%a - MMIO value for 0x%llx that was read back is 0x%x\n", __FUNCTION__, Address, NewValue));
}

STATIC EFI_STATUS AppleSiliconPciePlatformDxeSetupPciePort(APPLE_PCIE_COMPLEX_INFO *PcieComplex, dt_node_t *SubNode, UINT32 PortIndex) {
  if(SubNode == NULL)
    return EFI_SUCCESS;
  APPLE_PCIE_DEVICE_PORT_INFO *PciePortInfo = AllocateZeroPool(sizeof(APPLE_PCIE_DEVICE_PORT_INFO));
  EFI_STATUS Status;
  // CONST INT32 *IndexPtr;
  UINT32 Index;
  // INT32 IndexLength;
  APPLE_PCIE_GPIO_DESC ResetGpioStruct;
  EMBEDDED_GPIO *GpioProtocol;
  UINTN RetrievedGpioValue;

  BOOLEAN RefClk0Acked;
  BOOLEAN RefClk1Acked;
  BOOLEAN PortReady;
  BOOLEAN LinkUp;
  UINT32 i = 0;
  
  //
  // GPIO setup.
  //

  //
  // Locate the EmbeddedGpio protocol.
  //
  Status = gBS->LocateProtocol(&gEmbeddedGpioProtocolGuid, NULL, (VOID **)&GpioProtocol);
  if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "%a: Couldn't find the Embedded GPIO protocol\n", __FUNCTION__));
      return Status;
  }

  //
  // From the root PCIe0 node, get the addresses of each of the ports.
  //

  Index = PortIndex;

  PciePortInfo->Complex = PcieComplex;
  PciePortInfo->DevicePortIndex = Index;
  PciePortInfo->PortSubNode = SubNode;

  Status = AppleSiliconPciePlatformDxeGetResetGpios(SubNode, Index, &ResetGpioStruct);

  DEBUG((DEBUG_INFO, "%a - Reset GPIO number is 0x%x\n", __FUNCTION__, ResetGpioStruct.GpioNum));
  PciePortInfo->ResetGpioDesc = ResetGpioStruct;

  //
  // the device's base address is in the PCIe 'reg' property, after the rc base.
  // this implementation is a total hack
  // to compensate for not having the openfirmware interfacing that Linux and U-Boot have while
  // using devicetree.
  //
  PciePortInfo->DeviceBaseAddress = PcieComplex->PortRegionBase[Index];

  PciePortInfo->DevicePhyBaseAddress = PcieComplex->RcRegionBase + CORE_PHY_DEFAULT_BASE(PciePortInfo->DevicePortIndex);

  DEBUG((DEBUG_INFO, "%a - PCIe device base address is 0x%llx, device PHY base address is 0x%llx\n", __FUNCTION__, PciePortInfo->DeviceBaseAddress, PciePortInfo->DevicePhyBaseAddress));
  //
  // Begin bringing up the PCIe device.
  //

  AppleSiliconPcieSetBits(PORT_APPCLK_EN, PciePortInfo->DeviceBaseAddress + PORT_APPCLK);

  //
  // Per Asahi U-Boot, this is to assert PERST#
  //
  Status = GpioProtocol->Get(GpioProtocol, ResetGpioStruct.GpioNum, &RetrievedGpioValue);
  DEBUG((DEBUG_INFO, "%a - GPIO value before asserting PERST# is 0x%llx\n", __FUNCTION__, RetrievedGpioValue));
  Status = GpioProtocol->Set(GpioProtocol, ResetGpioStruct.GpioNum, GPIO_MODE_OUTPUT_0);
  Status = GpioProtocol->Get(GpioProtocol, ResetGpioStruct.GpioNum, &RetrievedGpioValue);
  DEBUG((DEBUG_INFO, "%a - GPIO value after asserting PERST# is 0x%llx\n", __FUNCTION__, RetrievedGpioValue));

  //
  // Set up REFCLK
  //

  switch(PcdGet32(PcdAppleSocIdentifier)) {
    case 0x6020:
    case 0x6021:
    case 0x6022:
      //
      // no PHY_LANE_CTL on T602X SoCs
      //
      break;
    default:
      //
      // Set up PHY_LANE_CTL
      //
      AppleSiliconPcieSetBits(PHY_LANE_CTL_CFGACC, PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CTL);
      break;
  }
  AppleSiliconPcieSetBits(PHY_LANE_CFG_REFCLK0REQ, PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG);

  RefClk0Acked = (MmioRead32(PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG) & PHY_LANE_CFG_REFCLK0ACK) != 0;

  while (RefClk0Acked == FALSE)
  {
    MicroSecondDelay(100);
    RefClk0Acked = (MmioRead32(PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG) & PHY_LANE_CFG_REFCLK0ACK) != 0;
    if(RefClk0Acked == TRUE || i >= 500) {
      break;
    }
    i++; 
  }

  i = 0;

  if(RefClk0Acked == FALSE) {
    DEBUG((DEBUG_ERROR, "%a - REFCLK0ACK timed out or failed\n", __FUNCTION__));
    return EFI_TIMEOUT;
  }

  AppleSiliconPcieSetBits(PHY_LANE_CFG_REFCLK1REQ, PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG);

  RefClk1Acked = (MmioRead32(PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG) & PHY_LANE_CFG_REFCLK1ACK) != 0;
  while(RefClk1Acked == FALSE)
  {
    MicroSecondDelay(100);
    RefClk1Acked = (MmioRead32(PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG) & PHY_LANE_CFG_REFCLK1ACK) != 0;
    if(RefClk1Acked == TRUE || i >= 500) {
      break;
    }
    i++; 
  }

  i = 0;

  if(RefClk1Acked == FALSE) {
    DEBUG((DEBUG_ERROR, "%a - REFCLK1ACK timed out or failed\n", __FUNCTION__));
    return EFI_TIMEOUT;
  }
  
  switch(PcdGet32(PcdAppleSocIdentifier)) {
    case 0x6020:
    case 0x6021:
    case 0x6022:
      //
      // no PHY_LANE_CTL on T602X SoCs
      //
      break;
    default:
      AppleSiliconPcieClearBits(PHY_LANE_CTL_CFGACC, PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CTL);
      break;
  }

  AppleSiliconPcieSetBits(PHY_LANE_CFG_REFCLKEN, PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG);

  switch(PcdGet32(PcdAppleSocIdentifier)) {
    case 0x6020:
    case 0x6021:
    case 0x6022:
      //
      // no PORT_REFCLK on T602X SoCs per Asahi driver.
      //
      break;
    default:
      AppleSiliconPcieSetBits(PORT_REFCLK_EN, PciePortInfo->DeviceBaseAddress + PORT_REFCLK);
      break;
  }

  //
  // Wait 100us for TPERST-CLK
  //
  MicroSecondDelay(100);

  //
  // de-assert PERST#
  //
  switch(PcdGet32(PcdAppleSocIdentifier)) {
    case 0x6020:
    case 0x6021:
    case 0x6022:
      AppleSiliconPcieSetBits(PORT_PERST_OFF, PciePortInfo->DeviceBaseAddress + PORT_T602X_PERST);
      break;
    default:
      AppleSiliconPcieSetBits(PORT_PERST_OFF, PciePortInfo->DeviceBaseAddress + PORT_PERST);
      break;
  }

  Status = GpioProtocol->Set(GpioProtocol, ResetGpioStruct.GpioNum, GPIO_MODE_OUTPUT_1);
  MicroSecondDelay(100 * 1000);

  PortReady = (MmioRead32(PciePortInfo->DeviceBaseAddress + PORT_STATUS) & PORT_STATUS_READY) != 0;
  while(PortReady == FALSE)
  {
    MicroSecondDelay(100);
    PortReady = (MmioRead32(PciePortInfo->DeviceBaseAddress + PORT_STATUS) & PORT_STATUS_READY) != 0;
    if(PortReady == TRUE || i >= 2500) {
      break;
    }
    i++;
  }

  i = 0;
  if(PortReady == FALSE) {
    DEBUG((DEBUG_ERROR, "%a - port %d ready signal timed out\n", __FUNCTION__, PciePortInfo->DevicePortIndex));
    return EFI_TIMEOUT;
  }

  DEBUG((DEBUG_INFO, "%a - port %d is ready\n", __FUNCTION__, PciePortInfo->DevicePortIndex));

  MmioWrite32(PciePortInfo->DeviceBaseAddress + PORT_LTSSMCTL, PORT_LTSSMCTL_START);

  LinkUp = (MmioRead32(PciePortInfo->DeviceBaseAddress + PORT_LINKSTS) & PORT_LINKSTS_UP) != 0;

  while(LinkUp == FALSE)
  {
    MicroSecondDelay(100);
    LinkUp = (MmioRead32(PciePortInfo->DeviceBaseAddress + PORT_LINKSTS) & PORT_LINKSTS_UP) != 0;
    if(LinkUp == TRUE || i >= 1000) {
      break;
    }
    i++;
  }
  i = 0;
  // if(LinkUp == FALSE) {
  //   DEBUG((DEBUG_ERROR, "%a - port %d link up timed out\n", __FUNCTION__, PciePortInfo->DevicePortIndex));
  //   return EFI_TIMEOUT;
  // }

  switch(PcdGet32(PcdAppleSocIdentifier)) {
    case 0x6020:
    case 0x6021:
    case 0x6022:
      AppleSiliconPcieSetBits(PHY_LANE_CFG_REFCLKCGEN, PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG);
      break;
    default:
      AppleSiliconPcieClearBits(PORT_REFCLK_CGDIS, PciePortInfo->DeviceBaseAddress + PORT_REFCLK);
      break;
  }

  AppleSiliconPcieClearBits(PORT_APPCLK_CGDIS, PciePortInfo->DeviceBaseAddress + PORT_APPCLK);
  
  DEBUG((DEBUG_INFO, "%a - PCIe port %d setup done\n", __FUNCTION__, PciePortInfo->DevicePortIndex));
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI 
AppleSiliconPciPlatformDxeInitialize(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
) 
{
    dt_node_t *ApcieNode = dt_get("apcie");
    APPLE_PCIE_COMPLEX_INFO *PcieComplexInfoStruct = AllocateZeroPool(sizeof(APPLE_PCIE_COMPLEX_INFO));
    CHAR8 PcieBridgeName[31];
    // CONST CHAR8 *PortStatus;
    // INT32 PortStatusLength;
    dt_node_t *PcieSubNode;
    EFI_STATUS Status;
    UINT32 PciePortIndex = 0;

    //
    // The PCIe controller itself seems to be brought up by m1n1 itself, including tunables,
    // so we just need to bring up root ports here so that DXE can detect the devices on the ports, including the XHCI controller.
    //

    DEBUG((DEBUG_INFO, "%a - started\n", __FUNCTION__));

    //
    // Pull the ECAM base address from the FDT. Ditto for "rc" base address.
    // With the Asahi Linux FDTs, we can usually safely assume this to be the first reg entry,
    // but if this ever changes (as will be the case when booting right from iBoot using ADT), this approach will need to change too.
    // TODO: see how to do this with ADT on embedded or non-m1n1 case.
    //

    // 00000090 06000000 00000010 00000000 0

    // 00000080 06000000 00400000 00000000 1

    // 00000880 06000000 00000900 00000000 2
    // 00000c80 06000000 00000200 00000000 3
    // 0000008c 06000000 00400000 00000000 4
    // 00c02b3d 00000000 00100000 00000000 5

    // 00000081 06000000 00800000 00000000 6
    // 00000181 06000000 00100000 00000000 7
    // 00400880 06000000 00400000 00000000 8
    // 00800c80 06000000 00800000 00000000 9
    // 00c00081 06000000 00400000 00000000 10

    // 00000082 06000000 00800000 00000000 11
    // 00000182 06000000 00100000 00000000 12
    // 00800880 06000000 00400000 00000000 13
    // 00000d80 06000000 00800000 00000000 14
    // 00c00082 06000000 00400000 00000000 15

    // 00000083 06000000 00800000 00000000 16
    // 00000183 06000000 00100000 00000000 17
    // 00c00880 06000000 00400000 00000000 18
    // 00800d80 06000000 00800000 00000000 19
    // 00c00083 06000000 00400000 00000000 20


    dt_node_reg(ApcieNode, 0, &PcieComplexInfoStruct->EcamCfgRegionBase, (UINTN*)&PcieComplexInfoStruct->EcamCfgRegionSize);
    DEBUG((DEBUG_INFO, "%a - PCIe ECAM base: 0x%llx, size 0x%x\n", __FUNCTION__, PcieComplexInfoStruct->EcamCfgRegionBase, PcieComplexInfoStruct->EcamCfgRegionSize));
    dt_node_reg(ApcieNode, 1, &PcieComplexInfoStruct->RcRegionBase, NULL);
    DEBUG((DEBUG_INFO, "%a - PCIe RC base: 0x%llx\n", __FUNCTION__, PcieComplexInfoStruct->RcRegionBase));
    AppleSiliconPciePlatformDxeSetupPciePort(PcieComplexInfoStruct, NULL, 0);

    //
    // Pull the base address for each port
    //
    for(UINT32 i = 0; i <= 3; i++) {
      dt_node_reg(ApcieNode, 6 + i * 5, &PcieComplexInfoStruct->PortRegionBase[i], NULL);//TODO: check and improve
      
      AsciiSPrint(PcieBridgeName, ARRAY_SIZE(PcieBridgeName), "pci-bridge%d", PciePortIndex);
      PcieSubNode = dt_get(PcieBridgeName);
      if(PcieSubNode == NULL)
        break;
      
      DEBUG((DEBUG_INFO, "%a - PCIe port %d base: 0x%llx\n", __FUNCTION__, i, PcieComplexInfoStruct->PortRegionBase[i]));

      //
      // NOTE: the following does NOT account for APCIe-GE devices such as the Mac Pros.
      //
      DEBUG((DEBUG_INFO, "%a - starting PCIe port setup\n", __FUNCTION__));
      Status = AppleSiliconPciePlatformDxeSetupPciePort(PcieComplexInfoStruct, PcieSubNode, PciePortIndex);
      if(EFI_ERROR(Status)) {
        DEBUG((DEBUG_ERROR, "%a - Port setup failed\n", __FUNCTION__));
        ASSERT_EFI_ERROR(Status);
      }
      DEBUG((DEBUG_ERROR, "%a - Port setup succeeded\n", __FUNCTION__));
      PciePortIndex++;
    }

    return EFI_SUCCESS;
}