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
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/PerformanceLib.h>
#include <Library/PrePiHobListPointerLib.h>
#include <Library/PrePiLib.h>
#include <Library/SerialPortLib.h>

VOID Main(IN VOID *StackBase, IN UINTN StackSize, IN VOID *DeviceTreePtr, IN VOID *UefiMemoryBase)
{
    EFI_HOB_HANDOFF_INFO_TABLE  *HobList;
    EFI_STATUS Status;

    UINTN UefiMemoryLength = FixedPcdGet32(PcdSystemMemoryUefiRegionSize);
    InitializeUART();

    DEBUG(
      (EFI_D_INFO | EFI_D_LOAD,
       "UEFI Memory Base = 0x%p, Size = 0x%llx, Stack Base = 0x%p, Stack "
       "Size = 0x%llx\n",
       UefiMemoryBase, UefiMemoryLength, StackBase, StackSize));

    DEBUG((DEBUG_INFO, "Setting up DXE Hand-Off Blocks.\n"));

    HobList = HobConstructor(
        UefiMemoryBase,
        UefiMemoryLength,
        UefiMemoryBase,
        StackBase
    );

    PrePeiSetHobList(HobList);

    //set up memory HOBs
    DEBUG((DEBUG_INFO, "Invalidating data cache.\n"));

    InvalidateDataCacheRange((VOID *)(UINTN)PcdGet64(PcdFdBaseAddress), PcdGet32(PcdFdSize));    

    DEBUG(
        (DEBUG_INFO, 
        "Beginning memory hand off block setup.\n"));
    Status = MemoryPeim(UefiMemoryBase, UefiMemoryLength);
    if(EFI_ERROR(Status))
    {
        DEBUG(
            (DEBUG_ERROR | DEBUG_INFO,
            "PrePi: Memory setup failed! Status: 0x%llx \n",
            Status)
            );
        DEBUG(
            (DEBUG_ERROR | DEBUG_INFO,
            "Looping forever, check serial log for failure point\n")
            );
        DEBUG(
            (DEBUG_ERROR | DEBUG_INFO,
            "Issue reboot command through m1n1 hypervisor shell or send VDM command to reboot\n")
            );
        CpuDeadLoop();
    }

    //set up stack and CPU HOBs
    DEBUG((DEBUG_INFO, "Building up Stack/CPU HOBs\n"));
    BuildStackHob((UINT64)StackBase, StackSize);
    BuildCpuHob(ArmGetPhysicalAddressBits(), PcdGet8(PcdPrePiCpuIoSize));

    DEBUG((DEBUG_INFO, "Setting boot mode to full configuration...\n"));
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
        (DEBUG_INFO,
        "Loading DXE Core now\n")
        );
    Status = LoadDxeCoreFromFv(NULL, 0);
    ASSERT_EFI_ERROR(Status);
    
    //if we reach here, something has *seriously* gone wrong
    CpuDeadLoop();
}

VOID CEntryPoint(IN VOID *StackBase, IN UINTN StackSize, IN VOID *DeviceTreePtr, IN VOID *UefiMemoryBase)
{
    Main(StackBase, StackSize, DeviceTreePtr, UefiMemoryBase);
}


UINT32 InitializeUART(VOID)
{
    SerialPortInitialize();
    DEBUG((DEBUG_INFO, "M1 Project Mu Firmware (arm64e), version 1.0-alpha\n"));
    DEBUG((DEBUG_INFO, "If you can see this message, this means the UART works!!!\n"));
    return EFI_SUCCESS;
}