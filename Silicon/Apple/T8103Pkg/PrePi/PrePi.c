/**
 * PrePi.c
 * 
 * Main implementation of SEC phase
 * 
 * PEI is not necessary in this implementation, so build up HOBs and jump right to DXE.
 * 
 * TODO: Adapt code to dynamically change PcdSystemMemorySize to use the values from FDT instead of
 * hardcoding values.
 * 
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 * 
 */


#include "PrePi.h"
#include <Pi/PiBootMode.h>
#include <Pi/PiHob.h>
#include <PiDxe.h>
#include <PiPei.h>
#include <Library/ArmLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugAgentLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PeCoffLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/PerformanceLib.h>
#include <Library/PrePiHobListPointerLib.h>
#include <Library/PrePiLib.h>
#include <Library/SerialPortLib.h>

UINT32 InitializeUART(VOID);

VOID EFIAPI ProcessLibraryConstructorList(VOID);

VOID Main(IN VOID *StackBase, IN UINTN StackSize, IN VOID *DeviceTreePtr, IN UINT64 UefiMemoryBase)
{
    EFI_HOB_HANDOFF_INFO_TABLE  *HobList;
    EFI_STATUS Status;

    UINTN UefiMemoryLength = FixedPcdGet32(PcdSystemMemoryUefiRegionSize);
    //InitializeUART();

    DEBUG(
        (EFI_D_INFO | EFI_D_LOAD,
        "Flattened Device Tree Pointer: 0x%p\n",
        DeviceTreePtr));

    DEBUG(
      (EFI_D_INFO | EFI_D_LOAD,
       "UEFI Memory Base = 0x%llx, Size = 0x%llx, Stack Base = 0x%p, Stack "
       "Size = 0x%llx\n",
       UefiMemoryBase, UefiMemoryLength, StackBase, StackSize));
    
    PatchPcdSet64(PcdFdtPointer, (UINT64)DeviceTreePtr);


    DEBUG((EFI_D_INFO | EFI_D_LOAD, "Setting up DXE Hand-Off Blocks.\n"));

    HobList = HobConstructor(
        (VOID *)UefiMemoryBase,
        UefiMemoryLength,
        (VOID *)UefiMemoryBase,
        StackBase
    );

    PrePeiSetHobList(HobList);

    //set up memory HOBs
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "Invalidating data cache.\n"));

    InvalidateDataCacheRange((VOID *)(UINTN)PcdGet64(PcdFdBaseAddress), PcdGet32(PcdFdSize));    

    DEBUG(
        (EFI_D_INFO | EFI_D_LOAD, 
        "Beginning memory hand off block setup.\n"));
    Status = MemoryPeim(UefiMemoryBase, UefiMemoryLength);
    if(EFI_ERROR(Status))
    {
        DEBUG(
            (DEBUG_ERROR | DEBUG_INFO | DEBUG_LOAD,
            "PrePi: Memory setup failed! Status: 0x%llx \n",
            Status)
            );
        DEBUG(
            (DEBUG_ERROR | DEBUG_INFO | DEBUG_LOAD,
            "Looping forever, check serial log for failure point\n")
            );
        DEBUG(
            (DEBUG_ERROR | DEBUG_INFO | DEBUG_LOAD,
            "Issue reboot command through m1n1 hypervisor shell or send VDM command to reboot\n")
            );
        CpuDeadLoop();
    }

    //set up stack and CPU HOBs
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "Building up Stack/CPU HOBs\n"));
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "Stack Base: 0x%llx, Stack Size: 0x%llx\n", (UINT64)StackBase, StackSize));
    BuildStackHob((UINT64)StackBase, StackSize);
    BuildCpuHob(ArmGetPhysicalAddressBits(), PcdGet8(PcdPrePiCpuIoSize));

    DEBUG((EFI_D_INFO | EFI_D_LOAD, "Setting boot mode to full configuration...\n"));
    SetBootMode(BOOT_WITH_FULL_CONFIGURATION);

    Status = PlatformPeim();
    ASSERT_EFI_ERROR(Status);

    // SEC phase needs to run library constructors by hand.
    ProcessLibraryConstructorList();

    // Assume the FV that contains the PI (our code) also contains a compressed
    // FV.
    Status = DecompressFirstFv();
    ASSERT_EFI_ERROR(Status);

    // Load the DXE Core and transfer control to it
    DEBUG(
        (EFI_D_INFO | EFI_D_LOAD,
        "Loading DXE Core now\n")
        );
    Status = LoadDxeCoreFromFv(NULL, 0);
    ASSERT_EFI_ERROR(Status);
    
    //if we reach here, something has *seriously* gone wrong
    CpuDeadLoop();
}

VOID CEntryPoint(IN VOID *StackBase, IN UINTN StackSize, IN VOID *DeviceTreePtr, IN UINT64 UefiMemoryBase)
{
    Main(StackBase, StackSize, DeviceTreePtr, UefiMemoryBase);
}


UINT32 InitializeUART(VOID)
{
    SerialPortInitialize();
    DEBUG(
        (EFI_D_INFO | EFI_D_LOAD, 
        "M1 Project Mu Firmware (arm64e), version 1.0-alpha\n")
        );
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "If you can see this message, this means the UART works!!!\n"));
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "Initial FD Base Address - 0x%llx\n", PcdGet64(PcdFdBaseAddress)));
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "Initial FV Base Address - 0x%llx\n", PcdGet64(PcdFvBaseAddress)));
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "Current FDT Pointer (Placeholder): 0x%llx\n", PcdGet64(PcdFdtPointer)));
    return EFI_SUCCESS;
}

VOID UARTRelocationDebugMessage(VOID)
{
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "New FD Base Address - 0x%llx\n", PcdGet64(PcdFdBaseAddress)));
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "New FV Base Address - 0x%llx\n", PcdGet64(PcdFvBaseAddress)));
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "Current FDT Pointer: 0x%llx\n", PcdGet64(PcdFdtPointer)));
    DEBUG((EFI_D_INFO | EFI_D_LOAD | EFI_D_ERROR, "New System RAM Base = 0x%llx\n", PcdGet64(PcdSystemMemoryBase)));
    DEBUG((EFI_D_INFO | EFI_D_LOAD | EFI_D_ERROR, "New System RAM Size = 0x%llx\n", PcdGet64(PcdSystemMemorySize)));
    return;

}

//borrowed from ArmVirtPkg/PrePi/PrePi.c
VOID
RelocatePeCoffImage (
  IN  EFI_PEI_FV_HANDLE         FwVolHeader,
  IN  PE_COFF_LOADER_READ_FILE  ImageRead
  )
{
  EFI_PEI_FILE_HANDLE           FileHandle;
  VOID                          *SectionData;
  PE_COFF_LOADER_IMAGE_CONTEXT  ImageContext;
  EFI_STATUS                    Status;

  FileHandle = NULL;
  Status     = FfsFindNextFile (
                 EFI_FV_FILETYPE_SECURITY_CORE,
                 FwVolHeader,
                 &FileHandle
                 );
  ASSERT_EFI_ERROR (Status);

  Status = FfsFindSectionData (EFI_SECTION_PE32, FileHandle, &SectionData);
  if (EFI_ERROR (Status)) {
    Status = FfsFindSectionData (EFI_SECTION_TE, FileHandle, &SectionData);
  }

  ASSERT_EFI_ERROR (Status);

  ZeroMem (&ImageContext, sizeof ImageContext);

  ImageContext.Handle    = (EFI_HANDLE)SectionData;
  ImageContext.ImageRead = ImageRead;
  PeCoffLoaderGetImageInfo (&ImageContext);

  if (ImageContext.ImageAddress != (UINTN)SectionData) {
    ImageContext.ImageAddress = (UINTN)SectionData;
    PeCoffLoaderRelocateImage (&ImageContext);
  }
}