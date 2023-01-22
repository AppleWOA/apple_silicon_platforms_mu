/**
 * @file BootRamdiskHelperDxe.c
 * @author amarioguy (Arminder Singh)
 * 
 * Sets up an embedded RAMDisk in the FV. For right now this is primarily used to bring up WinPE.
 * 
 * @version 1.0
 * @date 2022-12-25
 * 
 * @copyright Copyright (c) amarioguy (Arminder Singh), 2022.
 * 
 */

#include <PiDxe.h>

#include "BootRamdiskHelperDxe.h"

/**
 * @brief Main function for RAMDisk initialization.
 * 
 * Note that we only allocate a RAMDisk with a boot image here if we are told to do so and actually have the file embedded in the FV.
 * 
 * @param ImageHandle 
 * @param SystemTable 
 * @return
 * 
 * EFI_SUCCESS - we initialized the RAMDisk as a bootable device.
 * EFI_UNSUPPORTED - we are configured not to set up the RAMDisk.
 * EFI_NOT_FOUND - no FV embedded candidate image is present.
 * EFI_OUT_OF_RESOURCES - for some reason, we are out of memory and cannot create the ramdisk.
 * EFI_ABORTED - an unexpected error occurred.
 */
EFI_STATUS
EFIAPI
BootRamdiskHelperDxeInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
    EFI_STATUS Status;
    BOOLEAN RamdiskBootConfigured;
    VOID *OriginalRamDiskPtr;
    VOID *DestinationRamdiskPtr;
    UINTN RamDiskSize;
    EFI_GUID *RamDiskRegisterType = &gEfiVirtualCdGuid; //hardcode to ISO image for now
    EFI_RAM_DISK_PROTOCOL *RamdiskProtocol;
    EFI_DEVICE_PATH_PROTOCOL *DevicePath;

    DEBUG((DEBUG_INFO, "BootRamdiskHelperDxe started\n"));
    // Before proceeding to RAMDisk creation, check that we're configured to do so
    // and that we have a candidate image with which to create said RAMDisk.
    RamdiskBootConfigured = PcdGetBool(PcdInitializeRamdisk);
    if(!RamdiskBootConfigured) {
        DEBUG((DEBUG_ERROR, "BootRamdiskHelperDxe - FV not configured for ramdisk boot, exiting\n"));
        return EFI_UNSUPPORTED;
    }
    Status = GetSectionFromAnyFv(&gAppleSiliconPkgEmbeddedRamdiskGuid, EFI_SECTION_RAW, 0, &OriginalRamDiskPtr, &RamDiskSize);
    if(EFI_ERROR(Status)) {
        DEBUG((DEBUG_ERROR, "BootRamdiskHelperDxe - no FV embedded ramdisk, exiting\n"));
        return EFI_NOT_FOUND;
    }

    ASSERT (OriginalRamDiskPtr != NULL);
    ASSERT (RamDiskSize != 0);
    //copy the RAMDisk to a new scratch location
    DestinationRamdiskPtr = AllocateCopyPool(RamDiskSize, OriginalRamDiskPtr);

    ASSERT (DestinationRamdiskPtr != NULL);

    Status = gBS->LocateProtocol(&gEfiRamDiskProtocolGuid, NULL, (VOID **)&RamdiskProtocol);
    if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "BootRamdiskHelperDxe: Couldn't find the RAMDisk protocol - %r\n", Status));
        return Status;
    }
    Status = RamdiskProtocol->Register((UINTN)DestinationRamdiskPtr, (UINT64)RamDiskSize, RamDiskRegisterType, NULL, &DevicePath);
    if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "BootRamdiskHelperDxe: Cannot register RAM Disk - %r\n", Status));
    }

    return Status;

}
