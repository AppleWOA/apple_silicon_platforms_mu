/**
 * Copyright (c) 2023, amarioguy (AppleWOA authors).
 * 
 * Module Name:
 *     AppleEmbeddedGpioControllerDxe.c
 * 
 * Abstract:
 *     Platform specific driver for Apple silicon platforms to control the pin/GPIO controller.
 *     Based on the PL061 ARM GPIO DXE driver. (from ArmPlatformPkg)
 * 
 * Environment:
 *     UEFI DXE (Driver Execution Environment).
 * 
 * License:
 *     SPDX-License-Identifier: (BSD-2-Clause-Patent OR MIT) AND GPL-2.0
 * 
 *     Some code understanding is borrowed from the Asahi Linux project, copyright The Asahi Linux Contributors.
 * 
*/

#include <PiDxe.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Protocol/EmbeddedGpio.h>

#include "AppleEmbeddedGpioControllerDxe.h"

PLATFORM_GPIO_CONTROLLER *AppleSiliconPlatformGpioController;

//
// This driver's purpose is to control the GPIO controller on Apple platforms.
//

EFI_STATUS AppleEmbeddedGpioControllerLocate(IN EMBEDDED_GPIO_PIN Gpio, OUT UINTN *ControllerIndex, OUT UINTN *ControllerOffset, OUT UINTN *GpioRegisterBase) {
  for(UINT32 Index = 0; Index < AppleSiliconPlatformGpioController->GpioControllerCount; Index++) {
    if((Gpio >= AppleSiliconPlatformGpioController->GpioController[Index].GpioIndex) && (Gpio < AppleSiliconPlatformGpioController->GpioController[Index].GpioIndex + AppleSiliconPlatformGpioController->GpioController[Index].InternalGpioCount)) {
      *ControllerIndex = Index;
      *ControllerOffset = Gpio % AppleSiliconPlatformGpioController->GpioController[Index].InternalGpioCount;
      *GpioRegisterBase = AppleSiliconPlatformGpioController->GpioController[Index].RegisterBase;
      DEBUG((DEBUG_INFO, "%a - Found GPIO with index 0x%x, Offset 0x%llx, GPIO register base 0x%llx\n", __FUNCTION__, Index, (Gpio % AppleSiliconPlatformGpioController->GpioController[Index].InternalGpioCount), AppleSiliconPlatformGpioController->GpioController[Index].RegisterBase));
    }
    else {
      DEBUG((DEBUG_ERROR, "%a - GPIO 0x%x not found\n", Index));
      return EFI_NOT_FOUND;
    }
  }
}

EFI_STATUS EFIAPI AppleEmbeddedGpioGetGpioStatus(IN EMBEDDED_GPIO *This, IN EMBEDDED_GPIO_PIN Gpio, OUT UINTN *Value) {
  EFI_STATUS Status;
  UINTN Index;
  UINTN RegisterBase;
  UINTN Offset;
  Status = AppleEmbeddedGpioControllerLocate(Gpio, &Index, &Offset, &RegisterBase);
  if(EFI_ERROR(Status)) {
    DEBUG((DEBUG_INFO, "%a - GPIO not found\n", __FUNCTION__));
    ASSERT(FALSE);
  }
  if(Value == NULL) {
    DEBUG((DEBUG_INFO, "%a - cannot get status if output pointer is NULL\n", __FUNCTION__));
    return EFI_INVALID_PARAMETER;
  }
  
}

EFI_STATUS EFIAPI AppleEmbeddedGpioSetGpioStatus(IN EMBEDDED_GPIO *This, IN EMBEDDED_GPIO_PIN Gpio, IN EMBEDDED_GPIO_MODE Mode) {

}


EMBEDDED_GPIO  gGpio = {
  AppleEmbeddedGpioGetGpioStatus,
  AppleEmbeddedGpioSetGpioStatus,
  GetMode,
  SetPull
};


EFI_STATUS
EFIAPI 
AppleEmbeddedGpioControllerDxeInitialize(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
) 
{
  EFI_STATUS Status;
  EFI_HANDLE GpioHandle;
  GPIO_CONTROLLER *GpioController;
  UINTN NumberOfCpuDies;
  INT32 *NumberOfPinsNode;
  UINT64 FdtBlob = PcdGet64(PcdFdtPointer);
  INT32 PinCtrlApNodes[2];
  INT32 *PinCtrlApRegNode;
  UINT64 PinCtrlApReg;
  UINT32 Midr = ArmReadMidr();
  CHAR8 PinCtrlApNodeName[20]; //really 17 chars, but playing it safe here.
  BOOLEAN BaseSocSupportsMultipleDies; // T60XX SoCs support multiple dies, indicate that boolean here
  switch (PcdGet32(PcdAppleSocIdentifier)) {
    case 0x8103:
    case 0x8112:
    case 0x8122:
      NumberOfCpuDies = 1;
      BaseSocSupportsMultipleDies = FALSE;
      break;
    case 0x6000:
    case 0x6001:
    case 0x6020:
    case 0x6021:
    case 0x6030:
    case 0x6031:
    case 0x6034:
      NumberOfCpuDies = 1;
      BaseSocSupportsMultipleDies = TRUE;
      break;
    case 0x6002:
    case 0x6022:
      NumberOfCpuDies = 2;
      BaseSocSupportsMultipleDies = TRUE;
      break;
  }
  if(BaseSocSupportsMultipleDies == TRUE) {
    for(UINT32 i = 0; i < NumberOfCpuDies; i++) {
      AsciiSPrint(PinCtrlApNodeName, sizeof(PinCtrlApNodeName), "/die%d/pinctrl_ap", i);
      PinCtrlApNodes[i] = fdt_path_offset((VOID*)FdtBlob, PinCtrlApNodeName);
    }
  }
  else {
    PinCtrlApNodes[0] = fdt_path_offset((VOID*)FdtBlob, "/soc/pinctrl_ap");
  }
  //
  // Sanity check: make sure there isn't another GPIO controller check installed
  //
  ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gEmbeddedGpioProtocolGuid);

  Status = gBS->LocateProtocol (&gPlatformGpioProtocolGuid, NULL, (VOID **)&AppleSiliconPlatformGpioController);
  if(Status == EFI_NOT_FOUND) {
    // 
    // Allocate the Platform GPIO controller struct
    //
    AppleSiliconPlatformGpioController = (PLATFORM_GPIO_CONTROLLER *)AllocateZeroPool(sizeof (PLATFORM_GPIO_CONTROLLER) + sizeof (GPIO_CONTROLLER));
    if (AppleSiliconPlatformGpioController == NULL) {
      DEBUG((DEBUG_ERROR, "%a - allocation of platform GPIO controller struct failed\n", __FUNCTION__));
      return EFI_OUT_OF_RESOURCES;
    }
    //
    // Pin control GPIO controllers = 3 * NUM_DIES 
    // only 1 SMC pin control GPIO controller.
    // For now, use 1 * NUM_DIES (as for now we only need to concern ourselves with AP pin controllers)
    // if we need the others, change this
    //
    NumberOfPinsNode = fdt_getprop((VOID *)FdtBlob, PinCtrlApNodes[0], "apple,npins", NULL);
    AppleSiliconPlatformGpioController->GpioCount = NumberOfPinsNode[0];
    AppleSiliconPlatformGpioController->GpioControllerCount = (1 * NumberOfCpuDies);
    AppleSiliconPlatformGpioController->GpioController = (GPIO_CONTROLLER *)((UINTN)AppleSiliconPlatformGpioController + sizeof(PLATFORM_GPIO_CONTROLLER));

    GpioController = AppleSiliconPlatformGpioController->GpioController;
    GpioController->GpioIndex = 0;
    PinCtrlApRegNode = fdt_getprop((VOID *)FdtBlob, PinCtrlApNodes[0], "reg", NULL);
    PinCtrlApReg = fdt32_to_cpu(PinCtrlApRegNode[0]);
    PinCtrlApReg = (PinCtrlApReg << 32) | fdt32_to_cpu(PinCtrlApRegNode[1]);
    GpioController->RegisterBase = PinCtrlApReg;
    GpioController->InternalGpioCount = NumberOfPinsNode[0];

    //
    // Verify that we are on an Apple SoC by reading MIDR and checking the vendor ID bit,
    // bail out if not found.
    //
    if(((Midr >> 24) & 0x61) == 0) {
      DEBUG((DEBUG_ERROR, "%a - not on an Apple SoC, GPIO controller not present, aborting\n", __FUNCTION__));
      return EFI_NOT_FOUND;
    }
    //
    // Install the GPIO protocol.
    //
    GpioHandle = NULL;
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &GpioHandle,
                    &gEmbeddedGpioProtocolGuid,
                    &gGpio,
                    NULL
                    );
    if (EFI_ERROR (Status)) {
      DEBUG((DEBUG_ERROR, "%a - failed to install gEmbeddedGpioProtocolGuid, aborting\n", __FUNCTION__));
      Status = EFI_OUT_OF_RESOURCES;
    }

  }
}