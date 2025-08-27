/**
 * Copyright (c) 2023, amarioguy (AppleWOA authors).
 * 
 * Module Name:
 *     AppleSARTLib.c
 * 
 * Abstract:
 *     SART driver for Apple silicon platforms from Skye (A11) SoCs onwards.
 *     Required to bring up NVMe and permit DMA. Based off of the m1n1 and Linux driver.
 *     Currently the driver only supports Sicily (A14)/Tonga (M1) SoCs and newer.
 * 
 * Environment:
 *     UEFI DXE (Driver Execution Environment) and runtime services.
 * 
 * License:
 *     SPDX-License-Identifier: BSD-2-Clause-Patent OR MIT OR GPL-2.0-only.
 * 
 *     Original m1n1/Linux driver copyright (c) The Asahi Linux Contributors.
 * 
*/

#include <PiDxe.h>
#include <ConvenienceMacros.h>
#include <Base.h>
#include <Library/ArmLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/AppleAicLib.h>
#include <Library/AppleDTLib.h>

#define APPLE_SART_MAX_ENTRIES 16
#define SART_ALLOW_ALL_FLAG 0xff

typedef struct SART_DEVICE_STRUCT {
    //
    // base address of the SART.
    //
    UINTN SARTBaseAddress;
    UINT32 SARTProtectedEntries;
} SART_DEVICE;

//
// TODO: SARTv1 defs (for Skye/A11 support)
//


//
// Definitions that apply to either SART v2 or v3.
//

#define APPLE_SART_V2_V3_CONFIG(index) (0x0 + 4 * (index))
#define APPLE_SART_V2_V3_SIZE_SHIFT 12

//
// SART v2 specific definitions.
//

#define APPLE_SART_V2_CONFIG_FLAGS GENMASK(31, 24)
#define APPLE_SART_V2_PHYS_ADDR(index) (0x40 + 4 * (index))
#define APPLE_SART_V2_PHYS_ADDR_SHIFT APPLE_SART_V2_V3_SIZE_SHIFT
#define APPLE_SART_V2_CONFIG_SIZE_SHIFT APPLE_SART_V2_V3_SIZE_SHIFT
#define APPLE_SART_V2_CONFIG_SIZE GENMASK(23, 0)
#define APPLE_SART_V2_CONFIG_SIZE_MAX APPLE_SART_V2_CONFIG_SIZE

//
// SART v3 specific definitions.
//

#define APPLE_SART_V3_PHYS_ADDR(index) (0x40 + 4 * (index))
#define APPLE_SART_V3_PHYS_ADDR_SHIFT APPLE_SART_V2_V3_SIZE_SHIFT
#define APPLE_SART_V3_MAX_SIZE GENMASK(29, 0)
#define APPLE_SART_V3_SIZE(index) (0x80 + 4 * (index))

//
//  Description:
//    Gets a SARTv2 entry, and it's protection/DMA allowed status.
//
//  Return value:
//    None.
//
//  Notes:
//    Should only be called from the public AppleSARTGetEntry function.
//
STATIC VOID AppleSARTV2GetEntry(SART_DEVICE *SartDev, UINT32 Index, UINT8 *Flags, VOID **PhysAddr, size_t *Size) {
    UINT32 Config = MmioRead32(((SartDev->SARTBaseAddress) + APPLE_SART_V2_V3_CONFIG(Index)));
    *Flags = FIELD_GET(APPLE_SART_V2_CONFIG_FLAGS, Config);
    *Size = (size_t)((FIELD_GET(APPLE_SART_V2_CONFIG_SIZE, Config)) << APPLE_SART_V2_CONFIG_SIZE_SHIFT);
    *PhysAddr = (VOID *)((UINT64)(MmioRead32(((SartDev->SARTBaseAddress) + APPLE_SART_V2_PHYS_ADDR(Index))) << APPLE_SART_V2_PHYS_ADDR_SHIFT));
    DEBUG((DEBUG_INFO, "%a: SART entry %d has flags 0x%x, physical address 0x%p, size 0x%lx\n", __FUNCTION__, *Flags, *PhysAddr, *Size));
}

//
//  Description:
//    Sets a SARTv3 entry, and it's protection/DMA allowed status.
//
//  Return value:
//    TRUE if successful, FALSE otherwise.
//
//  Notes:
//    Should only be called from the public AppleSARTSetEntry function.
//
STATIC BOOLEAN AppleSARTV2SetEntry(SART_DEVICE *SartDev, UINT32 Index, UINT8 Flags, VOID *PhysAddr, size_t Size) {
    UINT32 Config;
    UINT64 PhysicalAddress = (UINT64)PhysAddr;

    if((Size & ((1 << APPLE_SART_V2_CONFIG_SIZE_SHIFT) - 1)) || (PhysicalAddress & ((1 << APPLE_SART_V2_PHYS_ADDR_SHIFT) - 1))) {
        DEBUG((DEBUG_INFO, "%a: ((Size & ((1 << APPLE_SART_V2_CONFIG_SIZE_SHIFT) - 1)) || (PhysicalAddress & ((1 << APPLE_SART_V2_PHYS_ADDR_SHIFT) - 1))) check hit, returning FALSE\n", __FUNCTION__));
        return FALSE;
    }
    Size = Size >> APPLE_SART_V2_CONFIG_SIZE_SHIFT;
    PhysicalAddress = PhysicalAddress >> APPLE_SART_V2_PHYS_ADDR_SHIFT;
    if(Size > APPLE_SART_V2_CONFIG_SIZE_MAX) {
        DEBUG((DEBUG_INFO, "%a: Size is greater than SARTv2 CONFIG_MAX_SIZE, returning FALSE\n"));
        return FALSE;
    }

    Config = FIELD_PREP(APPLE_SART_V2_CONFIG_FLAGS, Flags);
    Config |= FIELD_PREP(APPLE_SART_V2_CONFIG_SIZE, Size);
    DEBUG((DEBUG_INFO, "%a: Writing to SART at 0x%llx, index %d, with physical address 0x%llx, config flags 0x%d\n", __FUNCTION__, SartDev->SARTBaseAddress, Index, PhysicalAddress, Config));
    MmioWrite32(((SartDev->SARTBaseAddress) + APPLE_SART_V2_PHYS_ADDR(Index)), PhysicalAddress);
    MmioWrite32(((SartDev->SARTBaseAddress) + APPLE_SART_V2_V3_CONFIG(Index)), Config);

}


//
//  Description:
//    Gets a SARTv3 entry, and it's protection/DMA allowed status.
//
//  Return value:
//    None.
//
//  Notes:
//    Should only be called from the public AppleSARTGetEntry function.
//
STATIC VOID AppleSARTV3GetEntry(SART_DEVICE *SartDev, UINT32 Index, UINT8 *Flags, VOID **PhysAddr, size_t *Size) {
    *Flags = MmioRead32(((SartDev->SARTBaseAddress) + APPLE_SART_V2_V3_CONFIG(Index)));
    *Size = (size_t)(MmioRead32(((SartDev->SARTBaseAddress) + APPLE_SART_V3_SIZE(Index))) << APPLE_SART_V2_V3_SIZE_SHIFT);
    *PhysAddr = (VOID *)((UINT64)((MmioRead32(((SartDev->SARTBaseAddress) + APPLE_SART_V3_PHYS_ADDR(Index)))) << APPLE_SART_V3_PHYS_ADDR_SHIFT));
    DEBUG((DEBUG_INFO, "%a: SART entry %d has flags 0x%x, physical address 0x%p, size 0x%lx\n", __FUNCTION__, *Flags, *PhysAddr, *Size));
}

//
//  Description:
//    Sets a SARTv3 entry, and it's protection/DMA allowed status.
//
//  Return value:
//    TRUE if successful, FALSE otherwise.
//
//  Notes:
//    Should only be called from the public AppleSARTSetEntry function.
//
STATIC BOOLEAN AppleSARTV3SetEntry(SART_DEVICE *SartDev, UINT32 Index, UINT8 Flags, VOID *PhysAddr, size_t Size) {
    UINT64 PhysicalAddress = (UINT64)PhysAddr;
    if((Size & ((1 << APPLE_SART_V2_V3_SIZE_SHIFT) - 1)) || (PhysicalAddress & ((1 << APPLE_SART_V3_PHYS_ADDR_SHIFT) - 1))) {
        DEBUG((DEBUG_INFO, "%a: ((Size & ((1 << APPLE_SART_V2_V3_SIZE_SHIFT) - 1)) || (PhysicalAddress & ((1 << APPLE_SART_V3_PHYS_ADDR_SHIFT) - 1))) check hit, returning FALSE\n", __FUNCTION__));
        return FALSE;
    }
    PhysicalAddress = PhysicalAddress >> APPLE_SART_V3_PHYS_ADDR_SHIFT;
    Size = Size >> APPLE_SART_V2_V3_SIZE_SHIFT;

    if(Size > APPLE_SART_V3_MAX_SIZE) {
        DEBUG((DEBUG_INFO, "%a: Size is greater than SARTv3 MAX_SIZE, returning FALSE\n", __FUNCTION__));
        return FALSE;
    }
    DEBUG((DEBUG_INFO, "%a: Writing to SART at 0x%llx, index %d, with physical address 0x%llx, size 0x%x, config flags 0x%llx\n", __FUNCTION__, SartDev->SARTBaseAddress, Index, PhysicalAddress, Size, Flags));
    MmioWrite32(((SartDev->SARTBaseAddress) + APPLE_SART_V3_PHYS_ADDR(Index)), PhysAddr);
    MmioWrite32(((SartDev->SARTBaseAddress) + APPLE_SART_V3_SIZE(Index)), Size);
    MmioWrite32(((SartDev->SARTBaseAddress) + APPLE_SART_V2_V3_CONFIG(Index)), Flags);
    return TRUE;
}

//
//  Description:
//    Gets a SART entry, and it's protection/DMA allowed status.
//
//  Return value:
//    None.
//
VOID AppleSARTGetEntry(SART_DEVICE *SartDev, UINT32 Index, UINT8 *Flags, VOID **PhysAddr, size_t *Size) {
    UINT8 SARTVersion = FixedPcdGet8(PcdAppleSartVersion);
    switch(SARTVersion) {
        case 1:
            //
            // This is a SART v1, so we're most likely running on Skye (A11). (Not sure about Cyprus/A12 or Cebu/A13)
            // Support for this is unimplemented right now, but will be added later,
            // so for now indicate to the user that SARTv1 is unsupported but will be added.
            //
            // TODO: Add SARTv1 support.
            //
            DEBUG((DEBUG_ERROR | DEBUG_INFO, "%a: SARTv1 is unsupported for now, exiting\n", __FUNCTION__));
            break;
        //
        // This is a SARTv2 or SARTv3, call the appropriate function for the SART version.
        //
        case 2:
            DEBUG((DEBUG_INFO, "%a: Getting SARTv2 entry %d, Flags pointer %p, PhysAddr %p, Size pointer %p\n", __FUNCTION__, Index, Flags, *PhysAddr, Size));
            AppleSARTV2GetEntry(SartDev, Index, Flags, PhysAddr, Size);
            break;
        case 3:
            DEBUG((DEBUG_INFO, "%a: Getting SARTv3 entry %d, Flags pointer %p, PhysAddr %p, Size pointer %p\n", __FUNCTION__, Index, Flags, *PhysAddr, Size));
            AppleSARTV3GetEntry(SartDev, Index, Flags, PhysAddr, Size);
            break;
        default:
            //
            // Unrecognized SART, or SART version not set, inform the user.
            //
            DEBUG((DEBUG_ERROR, "%a: Unrecognized SART version, exiting\n", __FUNCTION__));
            break;
    }
}

BOOLEAN AppleSARTSetEntry(SART_DEVICE *SartDev, UINT32 Index, UINT8 Flags, VOID *PhysAddr, size_t Size) {
    UINT8 SARTVersion = FixedPcdGet8(PcdAppleSartVersion);
    switch(SARTVersion) {
        case 1:
            //
            // This is a SART v1, so we're most likely running on Skye (A11). (Not sure about Cyprus/A12 or Cebu/A13)
            // Support for this is unimplemented right now, but will be added later,
            // so for now indicate to the user that SARTv1 is unsupported but will be added.
            //
            // TODO: Add SARTv1 support.
            //
            DEBUG((DEBUG_ERROR | DEBUG_INFO, "%a: SARTv1 is unsupported for now, exiting\n", __FUNCTION__));
            break;
        //
        // This is a SARTv2 or SARTv3, call the appropriate function for the SART version.
        //
        case 2:
            DEBUG((DEBUG_INFO, "%a: Setting SARTv2 entry %d, Flags 0x%x, PhysAddr %p, Size 0x%x\n", __FUNCTION__, Index, Flags, PhysAddr, Size));
            AppleSARTV2SetEntry(SartDev, Index, Flags, PhysAddr, Size);
            break;
        case 3:
            DEBUG((DEBUG_INFO, "%a: Setting SARTv3 entry %d, Flags 0x%x, PhysAddr %p, Size 0x%x\n", __FUNCTION__, Index, Flags, PhysAddr, Size));
            AppleSARTV3SetEntry(SartDev, Index, Flags, PhysAddr, Size);
            break;
        default:
            //
            // Unrecognized SART, or SART version not set, inform the user.
            //
            DEBUG((DEBUG_ERROR, "%a: Unrecognized SART version, exiting\n", __FUNCTION__));
            break;
    }
}

EFI_STATUS EFIAPI AppleSARTLibInitialize(VOID) {
    UINT64 SARTBase = 0;
    SART_DEVICE *SartInstance;
    dt_node_t *SARTNode;
    UINT32 Length;
    UINT8 SARTVersion = FixedPcdGet8(PcdAppleSartVersion);
    //
    // Aside: why is the ANS controller on the second die on multi die systems...
    //

    SARTNode = dt_get("sart-ans");
    dt_node_reg(SARTNode, 0, &SARTBase, NULL);

    SartInstance = AllocateZeroPool(sizeof(SART_DEVICE));
    if(SartInstance == NULL) {
        DEBUG((DEBUG_INFO, "%a: Out of RAM, can't allocate space for SART device info, exiting\n", __FUNCTION__));
        return EFI_OUT_OF_RESOURCES;
    }
    SartInstance->SARTBaseAddress = SARTBase;
    DEBUG((DEBUG_INFO, "%a: SART version %d at 0x%llx\n", __FUNCTION__, SARTVersion, SARTBase));
    SartInstance->SARTProtectedEntries = 0;
    for(UINT32 i = 0; i < APPLE_SART_MAX_ENTRIES; i++) {
        VOID *PhysAddr;
        UINT8 Flags;
        size_t Size;

        AppleSARTGetEntry(SartInstance, i, &Flags, &PhysAddr, &Size);
        if(Flags) {
            SartInstance->SARTProtectedEntries |= (1 << i);
        }
    }
    return EFI_SUCCESS;

}