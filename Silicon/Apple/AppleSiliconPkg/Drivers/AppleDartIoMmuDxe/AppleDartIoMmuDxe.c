/**
 * Copyright (c) 2024, amarioguy (AppleWOA authors).
 * 
 * Module Name:
 *     AppleDartIoMmuDxe.c
 * 
 * Abstract:
 *     Platform specific driver for Apple silicon platforms to set up the DARTs.
 *     This driver depends on 16k page allocation working as it (hopefully) should.
 * 
 * 
 * Environment:
 *     UEFI DXE (Driver Execution Environment).
 * 
 * License:
 *     SPDX-License-Identifier: (BSD-2-Clause-Patent OR MIT) AND GPL-2.0
 * 
 *     Original code basis is from the Asahi Linux project fork of u-boot, original copyright and author notices below.
 *     Copyright (C) 2021 Mark Kettenis <kettenis@openbsd.org>
*/

#include <PiDxe.h>
#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/ArmLib.h>
#include <Library/PrintLib.h>
#include <Library/PcdLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/TimerLib.h>
#include <Include/libfdt.h>

#include <Protocol/IoMmu.h>

#include <Drivers/AppleDartIoMmuDxe.h>

APPLE_DART_INFO *DartInfo;

STATIC
PHYSICAL_ADDRESS
HostToDeviceAddress (
  IN  VOID  *Address
  )
{
  //return (PHYSICAL_ADDRESS)(UINTN)Address;
  return (PHYSICAL_ADDRESS)(UINTN)Address + PcdGet64 (PcdDmaDeviceOffset);
}

//
// Description:
//   Sets an attribute over memory that the DART manages.
//
// Return values:
//   EFI_SUCCESS - attribute is set successfully.
//

STATIC EFI_STATUS EFIAPI AppleDartIoMmuSetAttribute (
    IN EDKII_IOMMU_PROTOCOL *This, 
    IN EFI_HANDLE DeviceHandle, 
    IN VOID *Mapping, 
    IN UINT64 IoMmuAccess
    ) 
{
    //
    // Not sure of a way to do this on DART, for now, pass through the operation and return success.
    //
    return EFI_SUCCESS;
}

//
// Description:
//   Maps IOMMU managed memory so it becomes usable.
//
// Return values:
//   EFI_SUCCESS - mapped the range successfully.
//

STATIC EFI_STATUS EFIAPI AppleDartIoMmuMap(
    IN EDKII_IOMMU_PROTOCOL *This, 
    IN EDKII_IOMMU_OPERATION Operation, 
    IN VOID *HostAddress, 
    IN OUT UINTN *NumberOfBytes, 
    OUT EFI_PHYSICAL_ADDRESS *DeviceAddress, 
    OUT VOID **Mapping
    )
{

    PHYSICAL_ADDRESS PhysAddr, DmaVirtAddr;
    unsigned long PhysSize, Off;
    INT32 i, idx;
    APPLE_DART_MAPPING *DartMappingInfo;
    UINTN NumBytes = *NumberOfBytes;

    // DEBUG((DEBUG_INFO, "%a - mapping memory into IOMMU page tables, with host address 0x%p\n", __FUNCTION__, HostAddress));

    switch(Operation) {
        //
        // Treat everything as "common buffer" for now.
        //
        case EdkiiIoMmuOperationBusMasterRead:
        case EdkiiIoMmuOperationBusMasterRead64:
        case EdkiiIoMmuOperationBusMasterWrite:
        case EdkiiIoMmuOperationBusMasterWrite64:
        case EdkiiIoMmuOperationBusMasterCommonBuffer:
        case EdkiiIoMmuOperationBusMasterCommonBuffer64:
            //
            // if we are in bypass mode, just return the address verbatim.
            //
            if(DartInfo->BypassMode == TRUE) {
                *DeviceAddress = (PHYSICAL_ADDRESS)HostAddress;
                *Mapping = NULL; // nothing to map
                return EFI_SUCCESS;
            }

            //
            // since the USB-A controller doesn't support bypass mode, the below code MUST work correctly.
            //
            DartMappingInfo = AllocateZeroPool(sizeof(APPLE_DART_MAPPING));
            DartMappingInfo->HostAddr = (PHYSICAL_ADDRESS)HostAddress;
            PhysAddr = ALIGN_DOWN((PHYSICAL_ADDRESS)HostAddress, DART_PAGE_SIZE);
            DartMappingInfo->PhysAddress = PhysAddr;
            Off = (PHYSICAL_ADDRESS)HostAddress - PhysAddr;
            DartMappingInfo->Offset = Off;
            PhysSize = ALIGN(NumBytes + Off, DART_PAGE_SIZE);
            DartMappingInfo->PhysicalSize = PhysSize;
            DmaVirtAddr = (PHYSICAL_ADDRESS)HostAddress;
            DartMappingInfo->DmaVirtualAddr = DmaVirtAddr;

            DartMappingInfo->NumBytes = *NumberOfBytes;

            idx = DmaVirtAddr / DART_PAGE_SIZE;

            for(i = 0; i < PhysSize / DART_PAGE_SIZE; i++) {
                DartInfo->L2[idx + i] = (PhysAddr >> DartInfo->Shift) | DART_L2_VALID | DART_L2_START(0LL) | DART_L2_END(~0LL);
                PhysAddr += DART_PAGE_SIZE;
            }
            WriteBackInvalidateDataCacheRange((VOID *)DartInfo->L2[idx], ((UINTN)DartInfo->L2[idx + i] - (UINTN)DartInfo->L2[idx]));


            DartInfo->TlbFlush((VOID *)DartInfo);

            *DeviceAddress = HostToDeviceAddress((VOID *)DmaVirtAddr + Off);
            // DEBUG((DEBUG_INFO, "%a - device address is 0x%llx, Off = 0x%llx\n", __FUNCTION__, (DmaVirtAddr + Off), Off));
            *Mapping = DartMappingInfo;
            break;
        
        default:
            // if we have an invalid operation, just return
            break;

    }

    
    return EFI_SUCCESS;
}

//
// Description:
//   Unmaps IOMMU managed memory.
//
// Return values:
//   EFI_SUCCESS - unmapped the range successfully.
//

STATIC EFI_STATUS EFIAPI AppleDartIoMmuUnmap(IN EDKII_IOMMU_PROTOCOL *This, IN VOID *Mapping) {
    PHYSICAL_ADDRESS DmaVirtAddr;
    unsigned long PhysSize;
    INT32 i, idx;
    APPLE_DART_MAPPING *DartMappingInfo;

    DartMappingInfo = (APPLE_DART_MAPPING *)Mapping;

    // DEBUG((DEBUG_INFO, "%a - Unmapping memory from IOMMU page tables\n", __FUNCTION__));

    //
    // if we are in bypass mode, there is no conception of unmapping, we're done.
    //
    if(DartInfo->BypassMode == TRUE) {
        return EFI_SUCCESS;
    }

    //
    // since USB-A controller doesn't support bypass mode, the below code MUST work correctly.
    //

    DmaVirtAddr = ALIGN_DOWN(DartMappingInfo->PhysAddress, DART_PAGE_SIZE);

    PhysSize = (DartMappingInfo->NumBytes) + (DartMappingInfo->PhysAddress - DmaVirtAddr);
    PhysSize = ALIGN(PhysSize, DART_PAGE_SIZE);

    idx = DmaVirtAddr / DART_PAGE_SIZE;

    for(i = 0; i < PhysSize / DART_PAGE_SIZE; i++) {
        DartInfo->L2[idx + i] = DART_L2_INVAL;
    }

    WriteBackInvalidateDataCacheRange((VOID *)DartInfo->L2[idx], ((UINTN)DartInfo->L2[idx + i] - (UINTN)DartInfo->L2[idx]));

    DartInfo->TlbFlush((VOID *)DartInfo);
    FreeAlignedPages((VOID *)DmaVirtAddr, PhysSize);
    
    return EFI_SUCCESS;
}

//
// Description:
//   Allocates a DMA buffer for the IOMMU.
//
// Return values:
//   EFI_SUCCESS - allocated the buffer successfully.
//

STATIC EFI_STATUS EFIAPI AppleDartIoMmuAllocateBuffer (
    IN EDKII_IOMMU_PROTOCOL *This, 
    IN EFI_ALLOCATE_TYPE Type, 
    IN EFI_MEMORY_TYPE MemoryType, 
    IN UINTN Pages, 
    IN OUT VOID **HostAddress, 
    IN UINT64 Attributes
    )
{
    UINTN NewPages;
    NewPages = Pages;
    //
    // The only valid memory types are EfiBootServicesData and EfiRuntimeServicesData.
    // Currently copied from CoherentDmaLib, atm we're just hardcoding DART_PAGE_SIZE as alignment so it should all be good
    //

    //
    // HACK: if this is needed, then because EFI is always allocating pages in increments of 4k, always map 4 times the number
    // of pages requested, so that we always end up with 16k pages from the IOMMU perspective.
    //
    NewPages = 4 * Pages;
    // DEBUG((DEBUG_INFO, "%a - Allocating DMA buffer with %d pages\n", __FUNCTION__, NewPages));
    if (MemoryType == EfiBootServicesData) {
        *HostAddress = AllocateAlignedPages (NewPages, DART_PAGE_SIZE);
    } else if (MemoryType == EfiRuntimeServicesData) {
        *HostAddress = AllocateAlignedRuntimePages (NewPages, DART_PAGE_SIZE);
    } else {
        return EFI_INVALID_PARAMETER;
    }

    // DEBUG((DEBUG_INFO, "%a - HostAddress new value is 0x%p\n", __FUNCTION__, *HostAddress));

    if (*HostAddress == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }
    return EFI_SUCCESS;
}

STATIC EFI_STATUS EFIAPI AppleDartIoMmuFreeBuffer (
    IN EDKII_IOMMU_PROTOCOL *This,
    IN UINTN Pages,
    IN VOID *HostAddress
    )
{
    UINTN NewPages;
    NewPages = Pages;

    //
    // HACK: if this is needed, then because EFI is always allocating pages in increments of 4k, always map 4 times the number
    // of pages requested, so that we always end up with 16k pages from the IOMMU perspective.
    //
    NewPages = 4 * Pages;

    // DEBUG((DEBUG_INFO, "%a - Freeing DMA buffer at 0x%p, %d pages\n", __FUNCTION__, HostAddress, NewPages));
    if (HostAddress == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    FreeAlignedPages (HostAddress, NewPages);
    return EFI_SUCCESS;
}

EDKII_IOMMU_PROTOCOL mAppleDartIoMmuProtocol = {
    EDKII_IOMMU_PROTOCOL_REVISION,
    AppleDartIoMmuSetAttribute,
    AppleDartIoMmuMap,
    AppleDartIoMmuUnmap,
    AppleDartIoMmuAllocateBuffer,
    AppleDartIoMmuFreeBuffer,
};

STATIC VOID AppleDartT8020TlbFlush(VOID *DartInformation) {

    APPLE_DART_INFO *DartInfoStruct = (APPLE_DART_INFO *)DartInformation;
    __asm__("dsb sy");
    //SpeculationBarrier();
    MmioWrite32(DartInfoStruct->BaseAddress + DART_T8020_TLB_SIDMASK, DART_ALL_STREAMS(DartInfoStruct));
    MmioWrite32(DartInfoStruct->BaseAddress + DART_T8020_TLB_CMD, DART_T8020_TLB_CMD_FLUSH);
    while((MmioRead32(DartInfoStruct->BaseAddress + DART_T8020_TLB_CMD) & DART_T8020_TLB_CMD_BUSY) != 0) {
        continue;
    }
}

EFI_STATUS EFIAPI 
AppleDartIoMmuDxeInitialize(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
)
{
    UINT64 FdtBlob = PcdGet64(PcdFdtPointer);
    UINT32 Midr = ArmReadMidr();
    INT32 DartNode;
    UINT64 DartReg;
    CONST INT32 *DartRegNode;
    PHYSICAL_ADDRESS Address;
    PHYSICAL_ADDRESS L2;
    INT32 Ntte;
    INT32 NL1, NL2;
    INT32 sid, i;
    UINT32 Params2;
    //UINT32 Params4;

    DartInfo = AllocateZeroPool(sizeof(APPLE_DART_INFO));
    //
    // set up the IOMMU. for now, we should only be really setting up the DART for the USB controller,
    // but very likely that this will change in the future.
    //
    // may need to install multiple instances of the IOMMU protocol to control different DARTs?
    //

    //
    // HACK: for now, only set up the USB-A DART. this needs to change later on
    //

    DEBUG((DEBUG_INFO, "%a - started, FDT pointer: 0x%llx\n", __FUNCTION__, FdtBlob));

    //
    // Verify that we are on an Apple SoC by reading MIDR and checking the vendor ID bit,
    // bail out if not found.
    //
    if(((Midr >> 24) & 0x61) == 0) {
      DEBUG((DEBUG_ERROR, "%a - not on an Apple SoC, DARTs not present, aborting\n", __FUNCTION__));
      return EFI_NOT_FOUND;
    }

    //
    // Get the base address from the FDT
    //

    DartNode = fdt_path_offset((VOID*)FdtBlob, "/soc/iommu@584008000");
    DartRegNode = fdt_getprop((VOID *)FdtBlob, DartNode, "reg", NULL);
    DartReg = fdt32_to_cpu(DartRegNode[0]);
    DartReg = (DartReg << 32) | fdt32_to_cpu(DartRegNode[1]);

    DEBUG((DEBUG_INFO, "%a - DART base address is 0x%llx\n", __FUNCTION__, DartReg));

    DartInfo->BaseAddress = DartReg;

    if(fdt_node_check_compatible((VOID*)FdtBlob, DartNode, "apple,t8110-dart") == 0) {
        //
        // fill this in later
        //
    }
    else {
        DEBUG((DEBUG_INFO, "%a - Setting up T8020-compatible DART\n", __FUNCTION__));
        DartInfo->Nsid = 16;
        DartInfo->Nttbr = 4;
        DartInfo->SidEnableBase = DART_T8020_SID_ENABLE;
        DartInfo->TcrBase = DART_T8020_TCR_BASE;
        DartInfo->TcrTranslateEnable = DART_T8020_TCR_TRANSLATE_ENABLE;
        DartInfo->TcrBypass = (DART_T8020_TCR_BYPASS_DAPF | DART_T8020_TCR_BYPASS_DART);
        DartInfo->TtbrBase = DART_T8020_TTBR_BASE;
        DartInfo->TtbrIsValid = DART_T8020_TTBR_VALID;
        DartInfo->BypassMode = FALSE; // Assume there's no bypass mode by default.
        DartInfo->TlbFlush = AppleDartT8020TlbFlush;
    }

    if((fdt_node_check_compatible((VOID*)FdtBlob, DartNode, "apple,t8110-dart") == 0) || 
    (fdt_node_check_compatible((VOID*)FdtBlob, DartNode, "apple,t6000-dart") == 0)) {
        DartInfo->Shift = 4;
    }

    DartInfo->DmaVirtAddrBase = DART_PAGE_SIZE;
    DartInfo->DmaVirtAddrEnd = SIZE_4GB - DART_PAGE_SIZE;

    for(sid = 0; sid < DartInfo->Nsid; sid++) {
        MmioWrite32(DartInfo->BaseAddress + DART_TCR(DartInfo, sid), 0);
    }
    for(sid = 0; sid < DartInfo->Nsid; sid++) {
        for(i = 0; i < DartInfo->Nttbr; i++) {
            MmioWrite32(DartInfo->BaseAddress + DART_TTBR(DartInfo, sid, i), 0);
        }
    }

    DartInfo->TlbFlush((VOID *)DartInfo);

    Params2 = MmioRead32(DartInfo->BaseAddress + DART_PARAMS2);
    if((Params2 & DART_PARAMS2_BYPASS_SUPPORT) != 0) {
        for(sid = 0; sid < DartInfo->Nsid; sid++) {
            MmioWrite32(DartInfo->BaseAddress + DART_TCR(DartInfo, sid), DartInfo->TcrBypass);
        }
        DartInfo->BypassMode = TRUE;
        return EFI_SUCCESS;
    }

    //
    // if there's no bypass mode available for this DART, set up translation.
    // Warning: might not work for some devices given page size shenanigans
    //
    Ntte = DIV_ROUND_UP(DartInfo->DmaVirtAddrEnd, DART_PAGE_SIZE);
    NL2 = DIV_ROUND_UP(Ntte, DART_PAGE_SIZE / sizeof(UINT64));
    NL1 = DIV_ROUND_UP(NL2, DART_PAGE_SIZE / sizeof(UINT64));

    DartInfo->L2 = AllocateAlignedPages(NL2 * DART_PAGE_SIZE, DART_PAGE_SIZE);
    memset(DartInfo->L2, 0, NL2 * DART_PAGE_SIZE);
    WriteBackInvalidateDataCacheRange((VOID *)DartInfo->L2, NL2 * DART_PAGE_SIZE);

    DartInfo->L1 = AllocateAlignedPages(NL1 * DART_PAGE_SIZE, DART_PAGE_SIZE);
    memset(DartInfo->L1, 0, NL1 * DART_PAGE_SIZE);
    L2 = (PHYSICAL_ADDRESS)DartInfo->L2;
    for(i = 0; i < NL2; i++) {
        DartInfo->L1[i] = (L2 >> DartInfo->Shift) | DART_L1_TABLE;
        L2 += DART_PAGE_SIZE;
    }
    WriteBackInvalidateDataCacheRange((VOID *)DartInfo->L1, NL1 * DART_PAGE_SIZE);

    for(sid = 0; sid < DartInfo->Nsid; sid++) {
        Address = (PHYSICAL_ADDRESS)DartInfo->L1;
        for(i = 0; i < NL1; i++) {
            MmioWrite32(DartInfo->BaseAddress + DART_TTBR(DartInfo, sid, i), (Address >> DART_TTBR_SHIFT | DartInfo->TtbrIsValid));
            Address += DART_PAGE_SIZE;
        }
    }
    DartInfo->TlbFlush((VOID *)DartInfo);

    for(i = 0; i < DartInfo->Nsid / 32; i++) {
        MmioWrite32(DartInfo->BaseAddress + DART_SID_ENABLE(DartInfo, i), ~0);
    }

    for(sid = 0; sid < DartInfo->Nsid; sid++) {
        MmioWrite32(DartInfo->BaseAddress + DART_TCR(DartInfo, sid), DartInfo->TcrTranslateEnable);
    }

    return gBS->InstallMultipleProtocolInterfaces (
                    &ImageHandle,
                    &gEdkiiIoMmuProtocolGuid,
                    &mAppleDartIoMmuProtocol,
                    NULL
                    );

}