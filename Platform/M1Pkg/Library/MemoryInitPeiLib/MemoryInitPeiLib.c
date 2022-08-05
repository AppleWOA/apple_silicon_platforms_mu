/**
 * @file MemoryInitPeiLib.c
 * 
 * @author amarioguy (Arminder Singh)
 * 
 * This file implements page table setup, memory HOB setup, and MMU initialization.
 * Adapted from SurfaceDuoPkg/MemoryInitPeiLib.c
 * 
 * @version 1.0
 * @date 2022-07-31
 * 
 * @copyright Copyright (c) amarioguy (Arminder Singh) 2022.
 * 
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 * 
 **/

#include <PiPei.h>

#include <Library/ArmMmuLib.h>
#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>

//Device memory map configuration file for UEFI (this is to help with pagetable initialization)
#include <Configuration/DeviceMemoryMap.h>

VOID BuildMemoryTypeInformationHob(VOID);

STATIC VOID InitMmu(IN ARM_MEMORY_REGION_DESCRIPTOR *MemoryTable)
{
    VOID *MemoryTranslationTableBase;
    UINTN MemoryTranslationTableSize;
    RETURN_STATUS StatusCode;

    DEBUG(
        (DEBUG_INFO, 
        "MemoryInitPeiLib: Enabling MMU, Page Table Base: 0x%p, Page Table Size: 0x%p", 
        &MemoryTranslationTableBase, &MemoryTranslationTableBase)
        );
    StatusCode = ArmConfigureMmu(MemoryTable, &MemoryTranslationTableBase, &MemoryTranslationTableSize);

    if(EFI_ERROR(StatusCode))
    {
        DEBUG((DEBUG_ERROR | DEBUG_INFO, "MemoryInitPeiLib: MMU enable failed!! Status: %llx", StatusCode));
    }
}

STATIC
VOID AddHob(PARM_MEMORY_REGION_DESCRIPTOR_EX Desc)
{
  BuildResourceDescriptorHob(
      Desc->ResourceType, Desc->ResourceAttribute, Desc->Address, Desc->Length);

  if (Desc->ResourceType == EFI_RESOURCE_SYSTEM_MEMORY ||
      Desc->MemoryType == EfiRuntimeServicesData)
  {
    BuildMemoryAllocationHob(Desc->Address, Desc->Length, Desc->MemoryType);
  }
}

EFI_STATUS EFIAPI MemoryPeim(IN EFI_PHYSICAL_ADDRESS UefiMemoryBase, IN UINT64 UefiMemorySize)
{
    PARM_MEMORY_REGION_DESCRIPTOR_EX MemoryDescriptorEx = gDeviceMemoryDescriptorEx;
    ARM_MEMORY_REGION_DESCRIPTOR MemoryDescriptor[MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT];
    UINTN Index = 0;

    //assert that PcdSystemMemorySize is set
    ASSERT(PcdGet64(PcdSystemMemorySize) != 0);
    DEBUG(
        (DEBUG_INFO,
        "Setting up memory HOB with UEFI memory base @ 0x%llx, size 0x%llx",
        UefiMemoryBase,
        UefiMemorySize)
        );
    // Run through each memory descriptor
    while (MemoryDescriptorEx->Length != 0) {
        switch (MemoryDescriptorEx->HobOption) {
        case AddMem:
        case AddDev:
            AddHob(MemoryDescriptorEx);
            break;
        case NoHob:
        default:
            goto update;
    }

  update:
    ASSERT(Index < MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT);

    MemoryDescriptor[Index].PhysicalBase = MemoryDescriptorEx->Address;
    MemoryDescriptor[Index].VirtualBase  = MemoryDescriptorEx->Address;
    MemoryDescriptor[Index].Length       = MemoryDescriptorEx->Length;
    MemoryDescriptor[Index].Attributes   = MemoryDescriptorEx->ArmAttributes;

    Index++;
    MemoryDescriptorEx++;
  }

  // Last one (terminator)
  ASSERT(Index < MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT);
  MemoryDescriptor[Index].PhysicalBase = 0;
  MemoryDescriptor[Index].VirtualBase  = 0;
  MemoryDescriptor[Index].Length       = 0;
  MemoryDescriptor[Index].Attributes   = 0;
  
  DEBUG((DEBUG_INFO, "Configuring MMU now\n"));
  InitMmu(MemoryDescriptor);

  if (FeaturePcdGet(PcdPrePiProduceMemoryTypeInformationHob)) {
    // Optional feature that helps prevent EFI memory map fragmentation.
    BuildMemoryTypeInformationHob();
  }
  return EFI_SUCCESS;
}