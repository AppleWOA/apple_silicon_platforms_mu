/**
 * Copyright (c) 2023, amarioguy (AppleWOA authors).
 * 
 * Module Name:
 *     AppleBootTimeEmbeddedFirmwareHelperDxe.c
 * 
 * Abstract:
 *     Helper driver for Apple Silicon platforms to load firmware blobs required for onboard PCIe or
 *     platform devices.
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


//
// This driver's main purpose is to load firmware blobs embedded in the FV
// for testing purposes during Apple platform bringup, since Apple relies
// on the AP uploading firmware blobs to the IOPs/coprocessors, and NAND storage isn't guaranteed
// to be available. (might also be useful for the remote boot case on phones, where an
// Asahi Linux EFI system partition is not guaranteed to be available.)
//
// Right now, it only handles the USB firmware, but can be extended to other firmware blobs
// if it is deemed necessary.
//

EFI_STATUS
EFIAPI 
AppleBootTimeEmbeddedFirmwareHelperDxeInitialize(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
)
{
    EFI_STATUS Status;
    VOID *UsbFirmwarePointer;
    UINTN UsbFirmwareSize;
    BOOLEAN UsbFirmwareFound = FALSE;
    DEBUG((DEBUG_INFO, "%a: AppleBootTimeEmbeddedFirmwareHelperDxe started\n", __FUNCTION__));
    //
    // Load the section of the FV that contains the USB firmware blob.
    //
    Status = GetSectionFromAnyFv(&gAppleSiliconPkgEmbeddedUsbFirmwareGuid, EFI_SECTION_RAW, 0, &UsbFirmwarePointer, &UsbFirmwareSize);
    if(EFI_ERROR(Status)) {
        DEBUG((DEBUG_ERROR, "%a: loading FV embedded firmware failed, status %r, exiting\n", __FUNCTION__, Status));
        return EFI_NOT_FOUND;
    }
    UsbFirmwareFound = TRUE;
    if(UsbFirmwareFound == TRUE) {
        AppleAsmediaInitializeControllerAndBootFirmware();
    }
    return EFI_SUCCESS;

}