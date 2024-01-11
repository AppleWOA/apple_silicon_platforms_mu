/**
 * Copyright (c) 2023, amarioguy (AppleWOA authors).
 * 
 * Module Name:
 *     AppleEmbeddedGpioControllerDxe.c
 * 
 * Abstract:
 *     Platform specific driver for Apple silicon platforms to control the pin/GPIO controller.
 *     Based on the PL061 ARM GPIO DXE driver.
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

//
// This driver's purpose is to control the GPIO controller on Apple platforms.
//

EFI_STATUS
EFIAPI 
AppleEmbeddedGpioControllerDxeInitialize(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
) 
{
  
}