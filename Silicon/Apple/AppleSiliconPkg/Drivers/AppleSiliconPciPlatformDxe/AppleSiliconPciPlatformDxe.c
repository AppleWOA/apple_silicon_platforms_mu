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

STATIC EFI_STATUS AppleSiliconPciePlatformDxeGetResetGpios(INT32 SubNode, INT32 Index, APPLE_PCIE_GPIO_DESC *Desc) {
  UINT64 FdtBlob = PcdGet64(PcdFdtPointer);
  CONST INT32 *GpioCellsNode;
  INT32 PinCtrlApNode;
  UINT32 NumGpioCells;
  CONST INT32 *ResetGpiosPtr;
  INT32 ResetGpiosLength;
  

  //
  // HACK - currently the GPIO address needs to be specified by base address in devicetree (the aliases don't seem to work?)
  // ask marcan, et al how those aliases can be used
  //
  switch (PcdGet32(PcdAppleSocIdentifier)) {
    case 0x8103:
    case 0x8112:
      PinCtrlApNode = fdt_path_offset((VOID *)FdtBlob, "/soc/pinctrl@23c100000");
      break;
    case 0x8122:
    case 0x6000:
    case 0x6001:
    case 0x6002:
    case 0x6020:
    case 0x6021:
    case 0x6022:
      PinCtrlApNode = fdt_path_offset((VOID *)FdtBlob, "/soc/pinctrl@39b028000");
      break;
  }

  //
  // Find the number of GPIO cells (this has been 2 in most Asahi Linux device trees) (after sanity checking)
  //

  if ( (PinCtrlApNode == (-FDT_ERR_BADPATH)) 
  || (PinCtrlApNode == (-FDT_ERR_NOTFOUND)) 
  || (PinCtrlApNode == (-FDT_ERR_BADMAGIC)) 
  || (PinCtrlApNode == (-FDT_ERR_BADVERSION)) 
  || (PinCtrlApNode == (-FDT_ERR_BADSTATE)) 
  || (PinCtrlApNode == (-FDT_ERR_BADSTRUCTURE)) 
  || (PinCtrlApNode == (-FDT_ERR_TRUNCATED))
  )
  {
      DEBUG((DEBUG_ERROR, "FDT path offset finding failed!!\n"));
      DEBUG((DEBUG_ERROR, "Error code: 0x%llx\n", (-PinCtrlApNode)));
      ASSERT(PinCtrlApNode != (-FDT_ERR_BADPATH));
      ASSERT(PinCtrlApNode != (-FDT_ERR_NOTFOUND));
      ASSERT(PinCtrlApNode != (-FDT_ERR_BADMAGIC));
      ASSERT(PinCtrlApNode != (-FDT_ERR_BADVERSION));
      ASSERT(PinCtrlApNode != (-FDT_ERR_BADSTATE));
      ASSERT(PinCtrlApNode != (-FDT_ERR_BADSTRUCTURE));
      ASSERT(PinCtrlApNode != (-FDT_ERR_TRUNCATED));
  }

  //
  // This is probably just temporary debugging code, will be removed later.
  //
  
  GpioCellsNode = fdt_getprop((VOID *)FdtBlob, PinCtrlApNode, "#gpio-cells", NULL);
  NumGpioCells = fdt32_to_cpu(GpioCellsNode[0]);
  DEBUG((DEBUG_INFO, "%a - there are %d gpio-cells in the devicetree\n", __FUNCTION__, NumGpioCells));

  //
  // END temporary debug code
  //

  //
  // Get the reset GPIO values from the node.
  //
  ResetGpiosPtr = fdt_getprop((VOID *)FdtBlob, SubNode, "reset-gpios", &ResetGpiosLength);
  DEBUG((DEBUG_INFO, "%a - length of reset-gpios is 0x%x\n", __FUNCTION__, ResetGpiosLength));
  Desc->GpioNum = fdt32_to_cpu(ResetGpiosPtr[0]);
  Desc->GpioActivePolarity = fdt32_to_cpu(ResetGpiosPtr[1]);
  return EFI_SUCCESS;

}

STATIC EFI_STATUS AppleSiliconPciePlatformDxeSetupPciePort(APPLE_PCIE_COMPLEX_INFO *PcieComplex, INT32 SubNode, UINT32 PortIndex) {
  APPLE_PCIE_DEVICE_PORT_INFO *PciePortInfo = AllocateZeroPool(sizeof(APPLE_PCIE_DEVICE_PORT_INFO));
  EFI_STATUS Status;
  // CONST INT32 *IndexPtr;
  UINT32 Index;
  // INT32 IndexLength;
  UINT32 PortAppClkValue;
  APPLE_PCIE_GPIO_DESC ResetGpioStruct;
  EMBEDDED_GPIO *GpioProtocol;
  // UINT64 FdtBlob = PcdGet64(PcdFdtPointer);
  UINT32 PhyLaneCtl;
  UINT32 PhyLaneCfg;
  UINT32 PortRefClk;
  UINT32 PortPerst;
  BOOLEAN RefClkAcked = TRUE;
  BOOLEAN PortReady = TRUE;
  BOOLEAN LinkUp = TRUE;
  UINT32 Timeout = 0;
  
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
  DEBUG((DEBUG_INFO, "%a - reading PORT_APPCLK addr 0x%llx\n", __FUNCTION__, PciePortInfo->DeviceBaseAddress + PORT_APPCLK));
  PortAppClkValue = MmioRead32(PciePortInfo->DeviceBaseAddress + PORT_APPCLK);
  PortAppClkValue = (PortAppClkValue | PORT_APPCLK_EN);
  DEBUG((DEBUG_INFO, "%a - writing PORT_APPCLK addr 0x%llx\n", __FUNCTION__, PciePortInfo->DeviceBaseAddress + PORT_APPCLK));
  MmioWrite32(PciePortInfo->DeviceBaseAddress + PORT_APPCLK, PortAppClkValue);

  //
  // Per Asahi U-Boot, this is to assert PERST#
  //
  Status = GpioProtocol->Set(GpioProtocol, ResetGpioStruct.GpioNum, GPIO_MODE_OUTPUT_0);

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
      DEBUG((DEBUG_INFO, "%a - reading PHY_LANE_CTL addr 0x%llx\n", __FUNCTION__, PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CTL));
      PhyLaneCtl = MmioRead32(PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CTL);
      PhyLaneCtl = PhyLaneCtl | PHY_LANE_CTL_CFGACC;
      DEBUG((DEBUG_INFO, "%a - writing PHY_LANE_CTL addr 0x%llx\n", __FUNCTION__, PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CTL));
      MmioWrite32(PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CTL, PhyLaneCtl);
      break;
  }
  DEBUG((DEBUG_INFO, "%a - reading PHY_LANE_CFG addr 0x%llx\n", __FUNCTION__, PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG));
  PhyLaneCfg = MmioRead32(PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG);
  PhyLaneCfg = PhyLaneCfg | PHY_LANE_CFG_REFCLK0REQ;
  DEBUG((DEBUG_INFO, "%a - writing PHY_LANE_CFG addr 0x%llx\n", __FUNCTION__, PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG));
  MmioWrite32(PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG, PhyLaneCfg);

  DEBUG((DEBUG_INFO, "%a - reading PHY_LANE_CFG addr 0x%llx\n", __FUNCTION__, PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG));
  while(!(MmioRead32(PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG) & PHY_LANE_CFG_REFCLK0ACK))
  {
    MicroSecondDelay(100);
    Timeout += 100;
    if(Timeout >= 50000) {
      RefClkAcked = FALSE;
      break;
    }
  }

  Timeout = 0;

  if(RefClkAcked == FALSE) {
    DEBUG((DEBUG_ERROR, "%a - REFCLK0ACK timed out\n", __FUNCTION__));
    return EFI_TIMEOUT;
  }

  DEBUG((DEBUG_INFO, "%a - reading PHY_LANE_CFG addr 0x%llx\n", __FUNCTION__, PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG));
  PhyLaneCfg = MmioRead32(PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG);
  PhyLaneCfg = PhyLaneCfg | PHY_LANE_CFG_REFCLK1REQ;
  DEBUG((DEBUG_INFO, "%a - writing PHY_LANE_CFG addr 0x%llx\n", __FUNCTION__, PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG));
  MmioWrite32(PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG, PhyLaneCfg);


  DEBUG((DEBUG_INFO, "%a - reading PHY_LANE_CFG addr 0x%llx\n", __FUNCTION__, PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG));
  while(!(MmioRead32(PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG) & PHY_LANE_CFG_REFCLK1ACK))
  {
    MicroSecondDelay(100);
    Timeout += 100;
    if(Timeout >= 50000) {
      RefClkAcked = FALSE;
      break;
    }
  }

  Timeout = 0;

  if(RefClkAcked == FALSE) {
    DEBUG((DEBUG_ERROR, "%a - REFCLK1ACK timed out\n", __FUNCTION__));
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
      DEBUG((DEBUG_INFO, "%a - reading PHY_LANE_CTL addr 0x%llx\n", __FUNCTION__, PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CTL));
      PhyLaneCtl = MmioRead32(PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CTL);
      PhyLaneCtl = PhyLaneCtl & (~(PHY_LANE_CTL_CFGACC));
      DEBUG((DEBUG_INFO, "%a - writing PHY_LANE_CTL addr 0x%llx\n", __FUNCTION__, PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CTL));
      MmioWrite32(PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CTL, PhyLaneCtl);
      break;
  }

  PhyLaneCfg = MmioRead32(PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG);
  PhyLaneCfg = PhyLaneCfg | PHY_LANE_CFG_REFCLKEN;
  MmioWrite32(PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG, PhyLaneCfg);


  switch(PcdGet32(PcdAppleSocIdentifier)) {
    case 0x6020:
    case 0x6021:
    case 0x6022:
      //
      // no PORT_REFCLK on T602X SoCs per Asahi driver.
      //
      break;
    default:
      PortRefClk = MmioRead32(PciePortInfo->DeviceBaseAddress + PORT_REFCLK);
      PortRefClk = PortRefClk | PORT_REFCLK_EN;
      MmioWrite32(PciePortInfo->DeviceBaseAddress + PORT_REFCLK, PortRefClk);
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
      PortPerst = MmioRead32(PciePortInfo->DeviceBaseAddress + PORT_T602X_PERST);
      PortPerst = PortPerst | PORT_PERST_OFF;
      MmioWrite32(PciePortInfo->DeviceBaseAddress + PORT_T602X_PERST, PortPerst);
      break;
    default:
      PortPerst = MmioRead32(PciePortInfo->DeviceBaseAddress + PORT_PERST);
      PortPerst = PortPerst | PORT_PERST_OFF;
      MmioWrite32(PciePortInfo->DeviceBaseAddress + PORT_PERST, PortPerst);
      break;
  }

  Status = GpioProtocol->Set(GpioProtocol, ResetGpioStruct.GpioNum, GPIO_MODE_OUTPUT_1);
  MicroSecondDelay(100 * 1000);
  Timeout = 0;

  while(!(MmioRead32(PciePortInfo->DeviceBaseAddress + PORT_STATUS) & PORT_STATUS_READY))
  {
    MicroSecondDelay(100);
    Timeout += 100;
    if(Timeout >= 250000) {
      PortReady = FALSE;
      break;
    }
  }
  Timeout = 0;
  if(PortReady == FALSE) {
    DEBUG((DEBUG_ERROR, "%a - port %d ready signal timed out\n", __FUNCTION__, PciePortInfo->DevicePortIndex));
    return EFI_TIMEOUT;
  }

  MmioWrite32(PciePortInfo->DeviceBaseAddress + PORT_LTSSMCTL, PORT_LTSSMCTL_START);

  while(!(MmioRead32(PciePortInfo->DeviceBaseAddress + PORT_LINKSTS) & PORT_LINKSTS_UP))
  {
    MicroSecondDelay(100);
    Timeout += 100;
    if(Timeout >= 100000) {
      LinkUp = FALSE;
      break;
    }
  }

  switch(PcdGet32(PcdAppleSocIdentifier)) {
    case 0x6020:
    case 0x6021:
    case 0x6022:
      PhyLaneCfg = MmioRead32(PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG);
      PhyLaneCfg = PhyLaneCfg | PHY_LANE_CFG_REFCLKCGEN;
      MmioWrite32(PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG, PhyLaneCfg);
      break;
    default:
      PortRefClk = MmioRead32(PciePortInfo->DeviceBaseAddress + PORT_REFCLK);
      PortRefClk = PortRefClk & (~(PORT_REFCLK_CGDIS));
      MmioWrite32(PciePortInfo->DeviceBaseAddress + PORT_REFCLK, PortRefClk);
      break;
  }
  PortAppClkValue = MmioRead32(PciePortInfo->DeviceBaseAddress + PORT_APPCLK);
  PortAppClkValue = PortAppClkValue & (~(PORT_APPCLK_CGDIS));
  MmioWrite32(PciePortInfo->DeviceBaseAddress + PORT_APPCLK, PortAppClkValue);

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
    UINT64 FdtBlob = PcdGet64(PcdFdtPointer);
    INT32 Length;
    INT32 PcieNode = fdt_path_offset((VOID *)FdtBlob, "/soc/pcie");
    CONST INT32 *PcieRegs = fdt_getprop((VOID *)FdtBlob, PcieNode, "reg", &Length);
    APPLE_PCIE_COMPLEX_INFO *PcieComplexInfoStruct = AllocateZeroPool(sizeof(APPLE_PCIE_COMPLEX_INFO));
    // CONST CHAR8 *PortStatus;
    // INT32 PortStatusLength;
    INT32 PcieSubNode;
    EFI_STATUS Status;
    UINT32 PciePortIndex = 0;

    //
    // The PCIe controller itself seems to be brought up by m1n1 itself, including tunables,
    // so we just need to bring up root ports here so that DXE can detect the devices on the ports, including the XHCI controller.
    //

    DEBUG((DEBUG_INFO, "%a - started, FDT pointer: 0x%llx\n", __FUNCTION__, FdtBlob));

    //
    // Pull the ECAM base address from the FDT. Ditto for "rc" base address.
    // With the Asahi Linux FDTs, we can usually safely assume this to be the first reg entry,
    // but if this ever changes (as will be the case when booting right from iBoot using ADT), this approach will need to change too.
    // TODO: see how to do this with ADT on embedded or non-m1n1 case.
    //
    PcieComplexInfoStruct->EcamCfgRegionBase = fdt32_to_cpu(PcieRegs[0]);
    PcieComplexInfoStruct->EcamCfgRegionBase = ((PcieComplexInfoStruct->EcamCfgRegionBase) << 32) | (fdt32_to_cpu(PcieRegs[1]));
    PcieComplexInfoStruct->EcamCfgRegionSize = fdt32_to_cpu(PcieRegs[3]);
    DEBUG((DEBUG_INFO, "%a - PCIe ECAM base: 0x%llx, size 0x%x\n", __FUNCTION__, PcieComplexInfoStruct->EcamCfgRegionBase, PcieComplexInfoStruct->EcamCfgRegionSize));
    PcieComplexInfoStruct->RcRegionBase = fdt32_to_cpu(PcieRegs[4]);
    PcieComplexInfoStruct->RcRegionBase = ((PcieComplexInfoStruct->RcRegionBase) << 32) | (fdt32_to_cpu(PcieRegs[5]));
    DEBUG((DEBUG_INFO, "%a - PCIe RC base: 0x%llx\n", __FUNCTION__, PcieComplexInfoStruct->RcRegionBase));

    //
    // Pull the base address for each port
    //
    for(UINT32 i = 0; i <= 3; i++) {
      PcieComplexInfoStruct->PortRegionBase[i] = fdt32_to_cpu(PcieRegs[8 + 4*i]);
      PcieComplexInfoStruct->PortRegionBase[i] = ((PcieComplexInfoStruct->PortRegionBase[i]) << 32) | fdt32_to_cpu(PcieRegs[9 + 4*i]);
      DEBUG((DEBUG_INFO, "%a - PCIe port %d base: 0x%llx\n", __FUNCTION__, i, PcieComplexInfoStruct->PortRegionBase[i]));
      if((PcdGet32(PcdAppleSocIdentifier)) == 0x8103 && (i >= 2)) {
        break;
      }
    }
    //
    // NOTE: the following does NOT account for APCIe-GE devices such as the Mac Pros.
    //
    fdt_for_each_subnode(PcieSubNode, (VOID *)FdtBlob, PcieNode) {
      // DEBUG((DEBUG_INFO, "Test1\n"));
      // PortStatus = fdt_getprop((VOID *)FdtBlob, PcieSubNode, "status", &PortStatusLength);
      // DEBUG((DEBUG_INFO, "%a - PCIe port status is %s", __FUNCTION__, PortStatus));
      // if(!strcmp(PortStatus, "disabled")) {
      //   DEBUG((DEBUG_INFO, "PCIe port disabled, continuing\n"));
      //   continue;
      // }
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