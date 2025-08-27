/**
 * Copyright (c) 2024, amarioguy (AppleWOA authors).
 * 
 * Module Name:
 *     AppleUsbTypeCBringupDxe.c
 * 
 * Abstract:
 *     Platform specific driver for Apple silicon platforms to bring up the USB-C ports.
 * 
 * Environment:
 *     UEFI DXE (Driver Execution Environment).
 * 
 * License:
 *     SPDX-License-Identifier: (BSD-2-Clause-Patent OR MIT) AND GPL-2.0
 * 
 *     Original code basis is from the Asahi Linux u-boot project, original copyright and author notices below.
 *     Copyright (C) 2022 Mark Kettenis <kettenis@openbsd.org>
 *     Copyright (C) The Asahi Linux Contributors.
 *     
 *     Parts of DWC3 bringup code brought in from edk2-platforms, original copyright notice below.
 *     Copyright 2017, 2020 NXP
*/

#include <PiDxe.h>
#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/ArmLib.h>
#include <Library/PrintLib.h>
#include <Library/PcdLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/TimerLib.h>
#include <Library/NonDiscoverableDeviceRegistrationLib.h>
#include <Library/AppleDTLib.h>

#include <Drivers/AppleUsbTypeCBringupDxe.h>

//
// This driver is just a stub to bringup register the DWC3 controller(s) as a non-discoverable XHCI controller(s), which
// the built-in XHCI DXE driver should be able to bring up more or less normally.
// Note that the PHY will be doing USB 2.0 speeds, because iBoot only brings up the PHY to that state,
// and bringing up the PHY to USB 3.0 speed is not only unnecessary for Windows installation but requires much
// more complicated tunables, and a number of them from fuses.
//
// As for actual bringup of the DWC3 controllers, the device registration sequence should be almost exactly the same as that of the sequence used on NXP's UsbHcd driver,
// Only difference here is that we do more of those bringups. Note that the Synopsys bringup will be based on the sequence done in 
// u-boot's DWC3 driver to ensure Apple platform compatibility. (which the NXP bringup sequence is based off of.)
//

STATIC VOID Dwc3XhciSetBeatBurstLength(IN DWC3_CONTROLLER *Controller) {
  MmioAndThenOr32 ((UINTN)&Controller->GSBusCfg0, ~USB3_ENABLE_BEAT_BURST_MASK,
    USB3_ENABLE_BEAT_BURST);

  MmioOr32 ((UINTN)&Controller->GSBusCfg1, USB3_SET_BEAT_BURST_LIMIT);
}

STATIC VOID Dwc3SetFladj(IN DWC3_CONTROLLER *Controller, IN UINT32 Value) {
  MmioOr32 ((UINTN)&Controller->GFLAdj, GFLADJ_30MHZ_REG_SEL | GFLADJ_30MHZ (Value));
}

STATIC VOID Dwc3SetMode(IN DWC3_CONTROLLER *Controller, IN UINT32 Mode) {
  MmioAndThenOr32 ((UINTN)&Controller->GCtl, ~(DWC3_GCTL_PRTCAPDIR (DWC3_GCTL_PRTCAP_OTG)), DWC3_GCTL_PRTCAPDIR (Mode));
}


STATIC VOID Dwc3ControllerSoftReset(IN DWC3_CONTROLLER *Controller) {
  //
  // put the core in reset first.
  //
  MmioOr32((UINTN)&Controller->GCtl, DWC3_GCTL_CORESOFTRESET);

  //
  // Assert USB 2 and USB 3 PHY reset here.
  // There doesn't seem to be Apple-specific carveouts for USB2 or USB3 PHY reset only in u-boot so reset both as per canonical
  // DWC3 u-boot/NXP EDK2 implementation.
  //
  MmioOr32((UINTN)&Controller->GUsb3PipeCtl[0], DWC3_GUSB3PIPECTL_PHYSOFTRST);

  MmioOr32((UINTN)&Controller->GUsb2PhyCfg, DWC3_GUSB2PHYCFG_PHYSOFTRST);

  MemoryFence();

  MicroSecondDelay(100 * 1000);

  //
  // Clear USB 2 and USB 3 PHY reset.
  // Note that this doesn't actually bring up the USB 3 PHY, that's separate ATC setup which we're not doing here for USB 3.
  //

  MmioAnd32((UINTN)&Controller->GUsb3PipeCtl[0], ~DWC3_GUSB3PIPECTL_PHYSOFTRST);

  MmioAnd32 ((UINTN)&Controller->GUsb2PhyCfg, ~DWC3_GUSB2PHYCFG_PHYSOFTRST);

  MemoryFence();

  MicroSecondDelay(100 * 1000);

  //
  // PHYs are stable, take core out of reset.
  //

  MmioAnd32 ((UINTN)&Controller->GCtl, ~DWC3_GCTL_CORESOFTRESET);

}

STATIC EFI_STATUS Dwc3XhciCoreInit(IN DWC3_CONTROLLER *Controller)
{
  UINT32 Dwc3Revision;
  UINT32 Dwc3RegVal;
  UINTN Dwc3HwParams1Reg;
  Dwc3Revision = MmioRead32((UINTN)&Controller->GSnpsId);

  if((Dwc3Revision & DWC3_GSNPSID_MASK) != DWC3_SYNOPSYS_ALT_ID) {
    DEBUG((DEBUG_ERROR, "Dwc3XhciCoreInit: Revision 0x%x Not a Synopsys DWC3 core, aborting\n", Dwc3Revision));
    return EFI_NOT_FOUND;
  }

  //
  // soft reset the DWC3 here.
  //
  Dwc3ControllerSoftReset(Controller);

  Dwc3HwParams1Reg = MmioRead32((UINTN)&Controller->GHwParams1);

  Dwc3RegVal = MmioRead32((UINTN)&Controller->GCtl);
  Dwc3RegVal &= ~DWC3_GCTL_SCALEDOWN_MASK;
  Dwc3RegVal &= ~DWC3_GCTL_DISSCRAMBLE;

  if(DWC3_GHWPARAMS1_EN_PWROPT(Dwc3HwParams1Reg) == DWC3_GHWPARAMS1_EN_PWROPT_CLK) {
    Dwc3RegVal &= ~DWC3_GCTL_DSBLCLKGTNG;
  } else {
    DEBUG((DEBUG_INFO,"Dwc3XhciCoreInit: Power optimization unavailable\n"));
  }
  //
  // Both U-Boot and the NXP UsbHcd driver check for DWC3 errata on revisions < 1.90a - do likewise.
  //
  if((Dwc3Revision & DWC3_RELEASE_MASK) < DWC3_RELEASE_190a) {
    Dwc3RegVal |= DWC3_GCTL_U2RSTECN;
  }
  MmioWrite32((UINTN)&Controller->GCtl, Dwc3RegVal);

  return EFI_SUCCESS;
}


//
// This function actually brings up the DWC3 controller. The PHY is already set up by iBoot so we don't need
// to deal with that here.
//
NON_DISCOVERABLE_DEVICE_INIT 
EFIAPI 
AppleUsbTypeCBringupDxeInitializeUsbController(IN UINTN Dwc3ControllerBaseReg)
{
  EFI_STATUS Status;
  DWC3_CONTROLLER *Dwc3Controller;
  UINT32 Usb2PhyCfgReg;
  //
  // PHY reset/clock is brought up by iBoot, no need to do it here.
  //

  Dwc3Controller = (VOID *)(Dwc3ControllerBaseReg + DWC3_REG_OFFSET);

  Status = Dwc3XhciCoreInit(Dwc3Controller);
  if(EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "AppleUsbTypeCBringupDxeInitializeUsbController: USB controller init failed, status %r\n", Status));
    return (VOID *)EFI_DEVICE_ERROR;
  }

  //
  // the core is initialized at this point, U-Boot sets USB2 PHY config based on quirks in the device tree.
  // Since Apple platforms have none of those quirks defined in known device trees, just read and write back the PHY config to be safe.
  //
  Usb2PhyCfgReg = MmioRead32((UINTN)&Dwc3Controller->GUsb2PhyCfg[0]);
  MmioWrite32((UINTN)&Dwc3Controller->GUsb2PhyCfg[0], Usb2PhyCfgReg);

  //
  // Set the DWC3 to host mode.
  //
  Dwc3SetMode(Dwc3Controller, DWC3_GCTL_PRTCAP_HOST);

  //
  // Disabling this for now but per XHCI spec, this should be set per U-Boot comments? do this as a troubleshooting step if things don't work out. also seems to be set in the "core" dwc3 code in U-Boot.
  //
  Dwc3SetFladj(Dwc3Controller, GFLADJ_30MHZ_DEFAULT);
  Dwc3XhciSetBeatBurstLength(Dwc3Controller);
  return (VOID*)Status;
  
}


VOID 
EFIAPI 
AppleUsbTypeCBringupDxeBringupCallback(IN EFI_EVENT Event, IN VOID *Context)
{
  EFI_STATUS Status;
  UINT32 NumDwc3Controllers;
  UINT64 Dwc3ControllerBaseAddr;
  CHAR8 Dwc3RegNodeName[31];
  UINT32 Dwc3ControllerRegSize;
  //
  // Close the event so that we don't have duplicate events floating around.
  //
  gBS->CloseEvent(Event);

  DEBUG((DEBUG_INFO, "AppleUsbTypeCBringupDxeBringupCallback started\n"));

  NumDwc3Controllers = PcdGet32(PcdAppleNumDwc3Controllers);

  for(UINT32 Dwc3Index = 0; Dwc3Index < NumDwc3Controllers; Dwc3Index++) {
    if((Dwc3Index == 0) || (Dwc3Index == 2)) {
      //
      // skip DWC3 0, it seems to be in charge of the DFU port.
      //
      continue;
    }

    AsciiSPrint(Dwc3RegNodeName, ARRAY_SIZE(Dwc3RegNodeName), "usb-drd%d", Dwc3Index);
    dt_node_t *Dwc3Node = dt_get(Dwc3RegNodeName);
 
    dt_node_reg(Dwc3Node, 0, &Dwc3ControllerBaseAddr, NULL);

    Dwc3ControllerRegSize = 0x100000;//TODO: get from ADT
    DEBUG((DEBUG_INFO, "AppleUsbTypeCBringupDxeBringupCallback: DWC3_%d base address: 0x%llx, size = 0x%x\n", Dwc3Index, Dwc3ControllerBaseAddr, Dwc3ControllerRegSize));
    
    //
    // Register the controller as a non-registerable XHCI DMA-coherent controller. (All DMA on Apple systems must be cache-coherent)
    // Note: if this doesn't end up working, change the DMA type to non-coherent as one of the first steps to try.
    //
    Status = RegisterNonDiscoverableMmioDevice(NonDiscoverableDeviceTypeXhci, 
             NonDiscoverableDeviceDmaTypeCoherent,
             AppleUsbTypeCBringupDxeInitializeUsbController(Dwc3ControllerBaseAddr),
             NULL,
             1,
             Dwc3ControllerBaseAddr,
             Dwc3ControllerRegSize);
  }
  return;
} 


EFI_STATUS
EFIAPI 
AppleUsbTypeCBringupDxeInitialize(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
) 
{
    EFI_STATUS               Status;
    EFI_EVENT                EndOfDxeEvent;
    //
    // The UsbHcd code in edk2-platforms for NXP platforms (which also use DWC3 controllers) registers the initialization to take place at the end of DXE phase.
    // I'm not quite sure why this is the case, but to avoid problems, do likewise.
    //
    DEBUG((DEBUG_INFO, "AppleUsbTypeCBringupDxeInitialize started\n"));

    Status = gBS->CreateEventEx(EVT_NOTIFY_SIGNAL,
                                TPL_CALLBACK,
                                AppleUsbTypeCBringupDxeBringupCallback,
                                NULL,
                                &gEfiEndOfDxeEventGroupGuid,
                                &EndOfDxeEvent);

    return Status;
}

