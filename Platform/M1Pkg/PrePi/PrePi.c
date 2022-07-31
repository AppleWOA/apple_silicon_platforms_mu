/**
 * PrePi.c
 * 
 * Main implementation of SEC phase
 * 
 * PEI is not necessary in this implementation, so build up HOBs and jump right to DXE
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
    DEBUG((DEBUG_INFO, "M1 Project Mu Firmware (arm64e), version 1.0-alpha (thanks to Cristoh for the memes)\n"));
    DEBUG((DEBUG_INFO, "If you can see this message, this means the UART works!!!\n"));
    return EFI_SUCCESS;
}