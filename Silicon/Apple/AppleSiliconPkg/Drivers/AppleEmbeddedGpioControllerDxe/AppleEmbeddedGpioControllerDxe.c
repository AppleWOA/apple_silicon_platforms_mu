/**
 * Copyright (c) 2023, amarioguy (AppleWOA authors).
 * 
 * Module Name:
 *     AppleEmbeddedGpioControllerDxe.c
 * 
 * Abstract:
 *     Platform specific driver for Apple silicon platforms to control the pin/GPIO controller.
 *     Based on the PL061 ARM GPIO DXE driver. (from ArmPlatformPkg).
 *     GPIO hardware control based off the Asahi Linux driver.
 * 
 * Environment:
 *     UEFI DXE (Driver Execution Environment).
 * 
 * License:
 *     SPDX-License-Identifier: (BSD-2-Clause-Patent OR MIT) AND GPL-2.0
 * 
 *     Some code understanding is borrowed from the Asahi Linux project, Copyright (c) The Asahi Linux Contributors.
 * 
*/

#include <PiDxe.h>

#include <Library/BaseLib.h>
#include <Library/ArmLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Include/libfdt.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/TimerLib.h>

#include <Protocol/EmbeddedGpio.h>

#include "AppleEmbeddedGpioControllerDxe.h"

PLATFORM_GPIO_CONTROLLER *AppleSiliconPlatformGpioController;

//
// This driver's purpose is to control the GPIO controller on Apple platforms.
// In Apple chips, at least as of Tonga/T8103, the pin controller is the same as the GPIO
// controller, and the pin mappings are one to one.
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
  return EFI_SUCCESS;
}

STATIC UINTN EFIAPI AppleEmbeddedGpioControllerGetRegister(IN UINTN RegisterBase, IN UINTN Offset) {
  DEBUG((DEBUG_INFO, "%a - doing MMIO read\n", __FUNCTION__));
  return MmioRead32(RegisterBase + GPIO_REG(Offset));
}

STATIC VOID EFIAPI AppleEmbeddedGpioControllerSetRegister(IN UINTN RegisterBase, IN UINTN Offset, IN UINTN Bitmask, IN UINTN Value) {
  UINTN OriginalValue = MmioRead32(RegisterBase + GPIO_REG(Offset));
  UINTN UpdatedValue = OriginalValue & ((~Bitmask));
  UpdatedValue = (UpdatedValue | Value);
  DEBUG((DEBUG_INFO, "%a - doing MMIO write of 0x%llx\n", __FUNCTION__, UpdatedValue));
  MmioWrite32((RegisterBase + GPIO_REG(Offset)), Value);
}

//
// Description:
//   Gets the given GPIO value.
//
// Return values:
//   EFI_SUCCESS - Got the value.
//   EFI_INVALID_PARAMETER - value output pointer is NULL.
//

EFI_STATUS EFIAPI AppleEmbeddedGpioGetGpio(IN EMBEDDED_GPIO *This, IN EMBEDDED_GPIO_PIN Gpio, OUT UINTN *Value) {
  EFI_STATUS Status;
  UINTN Index;
  UINTN RegisterBase;
  UINTN Offset;
  UINTN GpioValue;
  Status = AppleEmbeddedGpioControllerLocate(Gpio, &Index, &Offset, &RegisterBase);
  if(EFI_ERROR(Status)) {
    DEBUG((DEBUG_INFO, "%a - GPIO not found\n", __FUNCTION__));
    ASSERT(FALSE);
  }
  if(Value == NULL) {
    DEBUG((DEBUG_INFO, "%a - cannot get status if output pointer is NULL\n", __FUNCTION__));
    return EFI_INVALID_PARAMETER;
  }
  
  GpioValue = AppleEmbeddedGpioControllerGetRegister(RegisterBase, Offset);
  //*Value = !!((GpioValue & (GPIOx_REG_DATA)));
  *Value = GpioValue;
  return EFI_SUCCESS;

}

//
// Description:
//   Sets the EMBEDDED_GPIO_MODE (or direction) and output value (if applicable) of the given GPIO.
//
// Return values:
//   EFI_SUCCESS - GPIO set successfully
//   EFI_UNSUPPORTED - the given direction is not supported. 
//
EFI_STATUS EFIAPI AppleEmbeddedGpioSetGpio(IN EMBEDDED_GPIO *This, IN EMBEDDED_GPIO_PIN Gpio, IN EMBEDDED_GPIO_MODE Mode) {
  EFI_STATUS Status;
  UINTN Index;
  UINTN RegisterBase;
  UINTN Offset;
  Status = AppleEmbeddedGpioControllerLocate(Gpio, &Index, &Offset, &RegisterBase);
  if(EFI_ERROR(Status)) {
    DEBUG((DEBUG_INFO, "%a - GPIO not found\n", __FUNCTION__));
    ASSERT(FALSE);
  }
  //
  // Only input/output directions are supported, anything else
  // will return as unsupported.
  //
  switch(Mode) {
    case GPIO_MODE_INPUT:
      AppleEmbeddedGpioControllerSetRegister(RegisterBase, Offset, (GPIOx_PERIPHERAL | GPIOx_MODE | GPIOx_INPUT_ENABLE | GPIOx_REG_DATA), ((FIELD_PREP(GPIOx_MODE, GPIOx_IN_MODE_IRQ_OFF)) | (GPIOx_INPUT_ENABLE)));
      break;
    case GPIO_MODE_OUTPUT_0:
      AppleEmbeddedGpioControllerSetRegister(RegisterBase, Offset, (GPIOx_PERIPHERAL | GPIOx_MODE | GPIOx_INPUT_ENABLE | GPIOx_REG_DATA), ((FIELD_PREP(GPIOx_MODE, GPIOx_OUT_MODE))));
      break;
    case GPIO_MODE_OUTPUT_1:
      AppleEmbeddedGpioControllerSetRegister(RegisterBase, Offset, (GPIOx_PERIPHERAL | GPIOx_MODE | GPIOx_INPUT_ENABLE | GPIOx_REG_DATA), ((FIELD_PREP(GPIOx_MODE, GPIOx_OUT_MODE)) | (GPIOx_REG_DATA)));
      break;
    default:
      DEBUG((DEBUG_INFO, "%a - requested GPIO mode is unsupported\n", __FUNCTION__));
      return EFI_UNSUPPORTED;
  }
  return EFI_SUCCESS;

}

EFI_STATUS EFIAPI AppleEmbeddedGpioControllerGetMode(IN EMBEDDED_GPIO *This, IN EMBEDDED_GPIO_PIN Gpio, OUT EMBEDDED_GPIO_MODE *Mode) {
  EFI_STATUS Status;
  UINTN Index;
  UINTN RegisterBase;
  UINTN Offset;
  UINTN GpioValue;
  Status = AppleEmbeddedGpioControllerLocate(Gpio, &Index, &Offset, &RegisterBase);
  if(EFI_ERROR(Status)) {
    DEBUG((DEBUG_INFO, "%a - GPIO not found\n", __FUNCTION__));
    ASSERT(FALSE);
  }
  if(Mode == NULL) {
    DEBUG((DEBUG_ERROR, "%a - GPIO mode request cannot be NULL\n", __FUNCTION__));
    return EFI_INVALID_PARAMETER;
  }
  GpioValue = AppleEmbeddedGpioControllerGetRegister(RegisterBase, Offset);
  if (FIELD_GET(GPIOx_MODE, GpioValue) == GPIOx_OUT_MODE) {
    if((GpioValue & GPIOx_REG_DATA) != 0) {
      *Mode = GPIO_MODE_OUTPUT_1;
    }
    else {
      *Mode = GPIO_MODE_OUTPUT_0;
    }
  }
  else {
    *Mode = GPIO_MODE_INPUT;
  }
  return EFI_SUCCESS;

}

//
// Description:
//   Sets the pull up/pull down behavior of a GPIO. Not supported.
//
// Return values:
//   EFI_UNSUPPORTED - cannot do the operation as it's not supported.
//

EFI_STATUS EFIAPI AppleEmbeddedGpioControllerSetPull(IN EMBEDDED_GPIO *This, IN EMBEDDED_GPIO_PIN Gpio, IN EMBEDDED_GPIO_PULL Direction) {
  return EFI_UNSUPPORTED;
}


EMBEDDED_GPIO  gGpio = {
  AppleEmbeddedGpioGetGpio,
  AppleEmbeddedGpioSetGpio,
  AppleEmbeddedGpioControllerGetMode,
  AppleEmbeddedGpioControllerSetPull
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
  // UINTN NumberOfCpuDies;
  UINT8 NumberOfGpioPins = PcdGet8(PcdAppleNumGpios);
  UINT64 FdtBlob = PcdGet64(PcdFdtPointer);
  // INT32 PinCtrlApNodes[2];
  INT32 PinCtrlApNode;
  CONST INT32 *PinCtrlApRegNode;
  UINT64 PinCtrlApReg;
  UINT32 Midr = ArmReadMidr();
  //CHAR8 PinCtrlApNodeName[13];
  // BOOLEAN BaseSocSupportsMultipleDies; // T60XX SoCs support multiple dies, indicate that boolean here
  DEBUG((DEBUG_INFO, "%a - started, FDT pointer: 0x%llx\n", __FUNCTION__, FdtBlob));

  //
  // Sanity check: make sure there isn't another GPIO controller check installed
  //
  ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gEmbeddedGpioProtocolGuid);
  Status = gBS->LocateProtocol (&gPlatformGpioProtocolGuid, NULL, (VOID **)&AppleSiliconPlatformGpioController);
  if(EFI_ERROR (Status) && (Status == EFI_NOT_FOUND)) {
    DEBUG((DEBUG_INFO, "%a - embedded gpio protocol is not installed\n", __FUNCTION__));
    //
    // Verify that we are on an Apple SoC by reading MIDR and checking the vendor ID bit,
    // bail out if not found.
    //
    if(((Midr >> 24) & 0x61) == 0) {
      DEBUG((DEBUG_ERROR, "%a - not on an Apple SoC, GPIO controller not present, aborting\n", __FUNCTION__));
      return EFI_NOT_FOUND;
    }

    switch (PcdGet32(PcdAppleSocIdentifier)) {
      case 0x8103:
      case 0x8112:
      case 0x8122:
        // NumberOfCpuDies = 1;
        // BaseSocSupportsMultipleDies = FALSE;
        break;
      case 0x6000:
      case 0x6001:
      case 0x6020:
      case 0x6021:
      case 0x6030:
      case 0x6031:
      case 0x6034:
        // NumberOfCpuDies = 1;
        // BaseSocSupportsMultipleDies = TRUE;
        break;
      case 0x6002:
      case 0x6022:
        // NumberOfCpuDies = 2;
        // BaseSocSupportsMultipleDies = TRUE;
        break;
    }
    DEBUG((DEBUG_INFO, "%a - soc identified\n", __FUNCTION__));

    // if(BaseSocSupportsMultipleDies == TRUE) {
    //   for(UINT32 i = 0; i < NumberOfCpuDies; i++) {
    //     DEBUG((DEBUG_INFO, "%a - getting pinctrl_ap for die %d of 2\n", __FUNCTION__, i));
    //     //
    //     // HACK
    //     //
    //     if(i == 0) {
    //       PinCtrlApNodes[i] = fdt_path_offset((VOID*)FdtBlob, "/soc@200000000/pinctrl");
    //     }
    //     else if (i == 1) {
    //       PinCtrlApNodes[i] = fdt_path_offset((VOID*)FdtBlob, "/soc@2200000000/pinctrl");
    //     }
    //   }
    // }
    // else {
    DEBUG((DEBUG_INFO, "%a - getting pinctrl_ap for die 0\n", __FUNCTION__));
    //
    // TODO: change this to be SoC agnostic.
    //
    PinCtrlApNode = fdt_path_offset((VOID*)FdtBlob, "/soc/pinctrl@39b028000");
    // }
  
    // 
    // Allocate the Platform GPIO controller struct
    //
    DEBUG((DEBUG_ERROR, "%a - starting allocation of memory for platform gpio controller struct\n", __FUNCTION__));
    AppleSiliconPlatformGpioController = (PLATFORM_GPIO_CONTROLLER *)AllocateZeroPool (sizeof (PLATFORM_GPIO_CONTROLLER) + sizeof (GPIO_CONTROLLER));
    if (AppleSiliconPlatformGpioController == NULL) {
      DEBUG((DEBUG_ERROR, "%a - allocation of platform GPIO controller struct failed\n", __FUNCTION__));
      return EFI_OUT_OF_RESOURCES;
    }
    //
    // Pin control GPIO controllers = 3 * NUM_DIES 
    // only 1 SMC pin control GPIO controller.
    // For now, use 1 * NUM_DIES (as for now we only need to concern ourselves with AP pin controllers)
    // if we need the others, change this.
    //
    DEBUG((DEBUG_INFO, "%a - setting GPIO count to %d\n", __FUNCTION__, NumberOfGpioPins ));
    AppleSiliconPlatformGpioController->GpioCount = NumberOfGpioPins;
    DEBUG((DEBUG_INFO, "%a - setting GPIO controller count to 1\n", __FUNCTION__));
    AppleSiliconPlatformGpioController->GpioControllerCount = 1;
    // DEBUG((DEBUG_INFO, "%a - setting GPIO controller count to %d\n", __FUNCTION__, (1 * NumberOfCpuDies)));
    // AppleSiliconPlatformGpioController->GpioControllerCount = (1 * NumberOfCpuDies);
    DEBUG((DEBUG_INFO, "%a - setting GPIO controller pointer\n", __FUNCTION__));
    AppleSiliconPlatformGpioController->GpioController = (GPIO_CONTROLLER *)((UINTN)AppleSiliconPlatformGpioController + sizeof(PLATFORM_GPIO_CONTROLLER));
    //
    // HACK: only doing 1 cpu die support for now (die 0)
    //
    //DEBUG((DEBUG_INFO, "%a - setting GPIO pointer to 0x%llx\n", __FUNCTION__, ((GPIO_CONTROLLER *)((UINTN)AppleSiliconPlatformGpioController + sizeof(PLATFORM_GPIO_CONTROLLER)))));

    GpioController = AppleSiliconPlatformGpioController->GpioController;
    DEBUG((DEBUG_INFO, "%a - setting GPIO controller index (local var)\n", __FUNCTION__));
    GpioController->GpioIndex = 0;
    PinCtrlApRegNode = fdt_getprop((VOID *)FdtBlob, PinCtrlApNode, "reg", NULL);
    PinCtrlApReg = fdt32_to_cpu(PinCtrlApRegNode[0]);
    PinCtrlApReg = (PinCtrlApReg << 32) | fdt32_to_cpu(PinCtrlApRegNode[1]);
    DEBUG((DEBUG_INFO, "%a - setting GPIO controller base reg (local var) to 0x%llx\n", __FUNCTION__, PinCtrlApReg));
    GpioController->RegisterBase = PinCtrlApReg;
    GpioController->InternalGpioCount = NumberOfGpioPins;

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
  DEBUG((DEBUG_INFO, "%a - GPIO protocol installation successful\n", __FUNCTION__));
  return Status;
}