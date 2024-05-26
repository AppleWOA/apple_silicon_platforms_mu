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
  INT32 *GpioCellsNode;
  INT32 PinCtrlApNode;
  UINT32 NumGpioCells;
  INT32 *ResetGpiosPtr;
  UINT32 ResetGpiosLength;
  

  //
  // HACK - need to load the pinctrl_ap node from die 0 for multi-die
  // systems, right now use a switch statement based on global PCD, use /soc for
  // single die systems, /die0 for multi-die systems.
  //
  switch (PcdGet32(PcdAppleSocIdentifier)) {
    case 0x8103:
    case 0x8112:
    case 0x8122:
      PinCtrlApNode = fdt_path_offset((VOID *)FdtBlob, "/soc/pinctrl_ap");
      break;
    case 0x6000:
    case 0x6001:
    case 0x6002:
    case 0x6020:
    case 0x6021:
    case 0x6022:
    case 0x6030:
      PinCtrlApNode = fdt_path_offset((VOID *)FdtBlob, "/die0/pinctrl_ap");
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
      DEBUG((DEBUG_ERROR, "Error code: 0x%llx\n", (-InterruptControllerNode)));
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
  NumGpioCells = GpioCellsNode[0];
  DEBUG((DEBUG_INFO, "%a - there are %d gpio-cells in the devicetree\n", __FUNCTION__, NumGpioCells));

  //
  // END temporary debug code
  //

  //
  // Get the reset GPIO values from the node.
  //
  ResetGpiosPtr = fdt_getprop((VOID *)FdtBlob, SubNode, "reset-gpios", &ResetGpiosLength);
  DEBUG((DEBUG_INFO, "%a - length of reset-gpios is 0x%x\n", __FUNCTION__, ResetGpiosLength));
  Desc->GpioNum = ResetGpiosPtr[0];
  Desc->GpioActivePolarity = ResetGpiosPtr[1];
  return EFI_SUCCESS;

}

STATIC EFI_STATUS AppleSiliconPciePlatformDxeSetupPciePort(APPLE_PCIE_COMPLEX_INFO *PcieComplex, INT32 SubNode) {
  APPLE_PCIE_DEVICE_PORT_INFO *PciePortInfo = AllocateZeroPool(sizeof(APPLE_PCIE_DEVICE_PORT_INFO));
  EFI_STATUS Status;
  UINT32 *IndexPtr;
  UINT32 Index;
  UINT32 IndexLength;
  UINT32 Length;
  UINT32 PortAppClkValue;
  APPLE_PCIE_GPIO_DESC ResetGpioStruct;
  EMBEDDED_GPIO *GpioProtocol;
  UINT64 FdtBlob = PcdGet64(PcdFdtPointer);
  CHAR8 PortName[10];
  INT32 PcieNode = fdt_path_offset((VOID *)FdtBlob, "/soc/pcie");
  CONST INT32 *PcieRegs = fdt_getprop((VOID *)FdtBlob, PcieNode, "reg", &Length);
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
      DEBUG ((DEBUG_ERROR, "%a: Couldn't find the Embedded GPIO protocol - %r\n", __FUNCTION__, Status));
      return Status;
  }

  //
  // From the root PCIe0 node, get the addresses of each of the ports.
  //

  IndexPtr = fdt_getprop((VOID *)FdtBlob, SubNode, "reg", &IndexLength);
  Index = IndexPtr[0];
  Index = Index >> 11;

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
  PciePortInfo->DeviceBaseAddress = fdt32_to_cpu(PcieRegs[8 + 4*Index]);
  PciePortInfo->DeviceBaseAddress = (((PciePortInfo->DeviceBaseAddress) << 32) | fdt32_to_cpu(PcieRegs[9 + 4*Index]));

  PciePortInfo->DevicePhyBaseAddress = PcieComplex->RcRegionBase + CORE_PHY_DEFAULT_BASE(PciePortInfo->DevicePortIndex);

  //
  // Begin bringing up the PCIe device.
  //
  PortAppClkValue = MmioRead32(PciePortInfo->DeviceBaseAddress + PORT_APPCLK);
  PortAppClkValue = (PortAppClkValue | PORT_APPCLK_EN);
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
      PhyLaneCtl = MmioRead32(PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CTL);
      PhyLaneCtl = PhyLaneCtl | PHY_LANE_CTL_CFGACC;
      MmioWrite32(PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CTL, PhyLaneCtl);
      break;
  }

  PhyLaneCfg = MmioRead32(PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG);
  PhyLaneCfg = PhyLaneCfg | PHY_LANE_CFG_REFCLK0REQ;
  MmioWrite32(PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG, PhyLaneCfg);

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

  PhyLaneCfg = MmioRead32(PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG);
  PhyLaneCfg = PhyLaneCfg | PHY_LANE_CFG_REFCLK1REQ;
  MmioWrite32(PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CFG, PhyLaneCfg);


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
      PhyLaneCtl = MmioRead32(PciePortInfo->DevicePhyBaseAddress + PHY_LANE_CTL);
      PhyLaneCtl = PhyLaneCtl & (~(PHY_LANE_CTL_CFGACC));
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
    // Pull the base address for each port
    //
    for(UINT32 i = 0; i <= 3; i++) {
      PcieComplexInfoStruct->PortRegionBase[i] = fdt32_to_cpu(PcieRegs[8 + 4*i]);
      PcieComplexInfoStruct->PortRegionBase[i] = ((PcieComplexInfoStruct->PortRegionBase[i]) << 32) | fdt32_to_cpu(PcieRegs[9 + 4*i]);
      if((PcdGet32(PcdAppleSocIdentifier)) == 0x8103 && (i >= 2)) {
        break;
      }
    }
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