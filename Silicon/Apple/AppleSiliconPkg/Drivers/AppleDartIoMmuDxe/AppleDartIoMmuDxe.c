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
 *     =Copyright (C) The Asahi Linux Contributors.
*/

//
// Major refactor consideration: should we have an "AppleDartIoMmuLib" for common DART operations, with separate drivers for different device type DART management?
// We do need to consider bringing up other DARTs if we want to be considered feature complete.
//

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
#include <Library/AppleDTLib.h>

#include <Protocol/IoMmu.h>

#include <Drivers/AppleDartIoMmuDxe.h>

APPLE_DART_INFO DartInfo[FixedPcdGet32(PcdAppleNumDwc3Controllers) * 2];

// STATIC
// PHYSICAL_ADDRESS
// HostToDeviceAddress (
//   IN  VOID  *Address
//   )
// {
//   return (PHYSICAL_ADDRESS)(UINTN)Address;
// //   return (PHYSICAL_ADDRESS)(UINTN)Address + PcdGet64 (PcdDmaDeviceOffset);
// }

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

    // PHYSICAL_ADDRESS PhysAddr, DmaVirtAddr;
    // unsigned long PhysSize, Off;
    // INT32 i, idx;
    // APPLE_DART_MAPPING *DartMappingInfo;
    // UINTN NumBytes = *NumberOfBytes;

    // DEBUG((DEBUG_INFO, "%a - mapping memory into IOMMU page tables, with host address 0x%p\n", __FUNCTION__, HostAddress));

    // switch(Operation) {
    //     //
    //     // Treat everything as "common buffer" for now.
    //     //
    //     case EdkiiIoMmuOperationBusMasterRead:
    //     case EdkiiIoMmuOperationBusMasterRead64:
    //     case EdkiiIoMmuOperationBusMasterWrite:
    //     case EdkiiIoMmuOperationBusMasterWrite64:
    //     case EdkiiIoMmuOperationBusMasterCommonBuffer:
    //     case EdkiiIoMmuOperationBusMasterCommonBuffer64:
    //         //
    //         // if we are in bypass mode, just return the address verbatim.
    //         //
    //         if(DartInfo->BypassMode == TRUE) {
    //             *DeviceAddress = (PHYSICAL_ADDRESS)HostAddress;
    //             *Mapping = NULL; // nothing to map
    //             return EFI_SUCCESS;
    //         }

    //         //
    //         // since the USB-A controller doesn't support bypass mode, the below code MUST work correctly.
    //         //
    //         DartMappingInfo = AllocateZeroPool(sizeof(APPLE_DART_MAPPING));
    //         DartMappingInfo->HostAddr = (PHYSICAL_ADDRESS)HostAddress;
    //         PhysAddr = ALIGN_DOWN((PHYSICAL_ADDRESS)HostAddress, DART_PAGE_SIZE);
    //         DartMappingInfo->PhysAddress = PhysAddr;
    //         Off = (PHYSICAL_ADDRESS)HostAddress - PhysAddr;
    //         DartMappingInfo->Offset = Off;
    //         PhysSize = ALIGN(NumBytes + Off, DART_PAGE_SIZE);
    //         DartMappingInfo->PhysicalSize = PhysSize;
    //         DmaVirtAddr = (PHYSICAL_ADDRESS)HostAddress;
    //         DartMappingInfo->DmaVirtualAddr = DmaVirtAddr;

    //         DartMappingInfo->NumBytes = *NumberOfBytes;

    //         idx = DmaVirtAddr / DART_PAGE_SIZE;

    //         for(i = 0; i < PhysSize / DART_PAGE_SIZE; i++) {
    //             DartInfo->L2[idx + i] = (PhysAddr >> DartInfo->Shift) | DART_L2_VALID | DART_L2_START(0LL) | DART_L2_END(~0LL);
    //             PhysAddr += DART_PAGE_SIZE;
    //         }
    //         WriteBackInvalidateDataCacheRange((VOID *)DartInfo->L2[idx], ((UINTN)DartInfo->L2[idx + i] - (UINTN)DartInfo->L2[idx]));


    //         DartInfo->TlbFlush((VOID *)DartInfo);

    //         *DeviceAddress = HostToDeviceAddress((VOID *)DmaVirtAddr + Off);
    //         DEBUG((DEBUG_INFO, "%a - device address is 0x%llx, Off = 0x%llx, DVA: 0x%llx\n", __FUNCTION__, (DmaVirtAddr + Off), Off, DmaVirtAddr));
    //         *Mapping = DartMappingInfo;
    //         break;
        
    //     default:
    //         // if we have an invalid operation, just return
    //         break;

    // }

    
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
    // PHYSICAL_ADDRESS DmaVirtAddr;
    // unsigned long PhysSize;
    // INT32 i, idx;
    // APPLE_DART_MAPPING *DartMappingInfo;

    // DartMappingInfo = (APPLE_DART_MAPPING *)Mapping;

    // // DEBUG((DEBUG_INFO, "%a - Unmapping memory from IOMMU page tables\n", __FUNCTION__));

    // //
    // // if we are in bypass mode, there is no conception of unmapping, we're done.
    // //
    // if(DartInfo->BypassMode == TRUE) {
    //     return EFI_SUCCESS;
    // }

    // //
    // // since USB-A controller doesn't support bypass mode, the below code MUST work correctly if that controller is on.
    // //

    // DmaVirtAddr = ALIGN_DOWN(DartMappingInfo->PhysAddress, DART_PAGE_SIZE);

    // PhysSize = (DartMappingInfo->NumBytes) + (DartMappingInfo->PhysAddress - DmaVirtAddr);
    // PhysSize = ALIGN(PhysSize, DART_PAGE_SIZE);

    // idx = DmaVirtAddr / DART_PAGE_SIZE;

    // for(i = 0; i < PhysSize / DART_PAGE_SIZE; i++) {
    //     DartInfo->L2[idx + i] = DART_L2_INVAL;
    // }

    // WriteBackInvalidateDataCacheRange((VOID *)DartInfo->L2[idx], ((UINTN)DartInfo->L2[idx + i] - (UINTN)DartInfo->L2[idx]));

    // DartInfo->TlbFlush((VOID *)DartInfo);
    // FreeAlignedPages((VOID *)DmaVirtAddr, PhysSize);
    
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
    // UINTN NewPages;
    // NewPages = Pages;
    // //
    // // The only valid memory types are EfiBootServicesData and EfiRuntimeServicesData.
    // // Currently copied from CoherentDmaLib, atm we're just hardcoding DART_PAGE_SIZE as alignment so it should all be good
    // //

    // //
    // // HACK: if this is needed, then because EFI is always allocating pages in increments of 4k, always map 4 times the number
    // // of pages requested, so that we always end up with 16k pages from the IOMMU perspective.
    // //
    // NewPages = 4 * Pages;
    // DEBUG((DEBUG_INFO, "%a - Allocating DMA buffer for IOMMU with %d pages\n", __FUNCTION__, NewPages));
    // if (MemoryType == EfiBootServicesData) {
    //     *HostAddress = AllocateAlignedPages (NewPages, DART_PAGE_SIZE);
    // } else if (MemoryType == EfiRuntimeServicesData) {
    //     *HostAddress = AllocateAlignedRuntimePages (NewPages, DART_PAGE_SIZE);
    // } else {
    //     return EFI_INVALID_PARAMETER;
    // }

    // DEBUG((DEBUG_INFO, "%a - HostAddress is 0x%p\n", __FUNCTION__, *HostAddress));

    // if (*HostAddress == NULL) {
    //     return EFI_OUT_OF_RESOURCES;
    // }
    return EFI_SUCCESS;
}

STATIC EFI_STATUS EFIAPI AppleDartIoMmuFreeBuffer (
    IN EDKII_IOMMU_PROTOCOL *This,
    IN UINTN Pages,
    IN VOID *HostAddress
    )
{
    // UINTN NewPages;
    // NewPages = Pages;

    // //
    // // HACK: if this is needed, then because EFI is always allocating pages in increments of 4k, always map 4 times the number
    // // of pages requested, so that we always end up with 16k pages from the IOMMU perspective.
    // //
    // NewPages = 4 * Pages;

    // // DEBUG((DEBUG_INFO, "%a - Freeing DMA buffer at 0x%p, %d pages\n", __FUNCTION__, HostAddress, NewPages));
    // if (HostAddress == NULL) {
    //     return EFI_INVALID_PARAMETER;
    // }

    // FreeAlignedPages (HostAddress, NewPages);
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

STATIC VOID AppleDartT8110TlbFlush(VOID *DartInformation) {
    APPLE_DART_INFO *DartInfoStruct = (APPLE_DART_INFO *)DartInformation;
    __asm__("dsb sy");
    MmioWrite32(DartInfoStruct->BaseAddress + DART_T8110_TLB_CMD_FLUSH_ALL, DART_T8110_TLB_CMD_FLUSH_ALL);
    while((MmioRead32(DartInfoStruct->BaseAddress + DART_T8110_TLB_CMD)) & DART_T8110_TLB_CMD_BUSY) {
        continue;
    }
}

EFI_STATUS EFIAPI 
AppleDartIoMmuDxeInitialize(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
)
{
    UINT32 Midr;
    dt_node_t *DartNode[FixedPcdGet32(PcdAppleNumDwc3Controllers)];
    UINT64 DartReg[FixedPcdGet32(PcdAppleNumDwc3Controllers) * 2];
    UINT32 DartIndex = 0;
    UINT32 Params4; // U-Boot does this
    // PHYSICAL_ADDRESS Address;
    // PHYSICAL_ADDRESS L2;
    // INT32 Ntte;
    // INT32 NL1, NL2;
    INT32 sid, i;
    UINT32 Params2;
    CHAR8 DartNodeName[33];
    // BOOLEAN DartFound = TRUE; // assume the DART exists to start.
    //UINT32 Params4;


    //
    // set up the IOMMU. for now, we should only be really setting up the DARTs for the USB controllers,
    // but very likely that this will change in the future.
    
    //
    // Each DWC3 has two separate DARTs to manage requests but because the DWC3 DARTs are able to work in bypass mode, we can simply set bypass mode in this driver for each DART,
    // and then simply not register the protocol for any of them. (UEFI assumes direct DMA access is possible if no IOMMU protocol is present)
    // Note: this will NOT work for the USB-A controller, as that's on the PCIe bus (whose DARTs do NOT work in bypass mode), but other devices have a one device to one DART relation.
    // Also this driver is NOT a secure driver by virtue of setting up bypass mode.
    //


    //
    // Verify that we are on an Apple SoC by reading MIDR and checking the vendor ID bit,
    // bail out if not found.
    //
    Midr = ArmReadMidr();
    if(((Midr >> 24) & 0x61) == 0) {
      DEBUG((DEBUG_INFO, "MIDR: 0x%x\n", Midr));
      DEBUG((DEBUG_ERROR, "%a - not on an Apple SoC, DARTs not present, aborting\n", __FUNCTION__));
      ASSERT(FALSE);
      return EFI_NOT_FOUND;
    }

    //
    // Get the base addresses from the ADT. Right now there's no good way I know of to test for:
    //   a. the number of dies a multi-die capable SoC has.
    //   b. how many DARTs a given SoC die has.
    // Given that this driver is mainly meant to bring up USB controller DARTs and was mostly designed for that purpose, for now
    // only bring up the USB controller DARTs (which the following switch statement will assume hardcoded knowledge of for each SoC for now)
    // TODO: make this a dynamic lookup to keep the code SoC-agnostic.
    //

    //
    // USB-A DARTs used to be brought up in an earlier version of this driver, however due to issues with getting devices to enumerate
    // on the USB-A ports on supported devices, that code is disabled for the moment.
    //

    for(INT32 DartNodeIndex = 0; DartNodeIndex < FixedPcdGet32(PcdAppleNumDwc3Controllers); DartNodeIndex++) {
        if((DartNodeIndex == 0) || (DartNodeIndex == 2)) {
            DEBUG((DEBUG_INFO, "Skipping DFU port DWC %d\n", DartNodeIndex));
            continue;
        }

        AsciiSPrint(DartNodeName, ARRAY_SIZE(DartNodeName), "dart-usb%d", DartNodeIndex);
        DartNode[DartNodeIndex] = dt_get(DartNodeName);

        if(DartNode[DartNodeIndex] == NULL) {
            DEBUG((DEBUG_INFO, "Did not find node %a\n\n", DartNodeName));
            continue;
        }

        dt_node_reg(DartNode[DartNodeIndex], 0, &DartReg[DartIndex++], NULL);
        DEBUG((DEBUG_INFO, "DART reg[0] for %a is 0x%llx \n", DartNodeName, DartReg[DartIndex - 1]));
        dt_node_reg(DartNode[DartNodeIndex], 1, &DartReg[DartIndex++], NULL);
        DEBUG((DEBUG_INFO, "DART reg[1] for %a is 0x%llx \n", DartNodeName, DartReg[DartIndex - 1]));
    }


    for(DartIndex = 0; DartIndex < FixedPcdGet32(PcdAppleNumDwc3Controllers) * 2; DartIndex++) {
        //
        // to avoid killing the serial console from UART proxy - leave darts for DFU ports alone.
        //
        if(DartIndex == 0 || DartIndex == 1 || DartIndex == 4 || DartIndex == 5) {
            DEBUG((DEBUG_INFO, "Skipping DFU port DART %d\n", DartIndex));
            continue;
        }
        //DEBUG((DEBUG_INFO, "Test0\n"));
        DartInfo[DartIndex].BaseAddress = DartReg[DartIndex];

        char *CompatibleStr;
        size_t CompatibleStrLength = 0;
        CompatibleStr = dt_node_prop(DartNode[DartIndex / 2], "compatible", &CompatibleStrLength);

        if( !AsciiStrCmp(CompatibleStr ,"dart,t8110")) {
            //
            // T8110 compatible DARTs have different setup.
            //
            DEBUG((DEBUG_INFO, "%a - Setting up T8110-compatible DART\n", __FUNCTION__));
            Params4 = MmioRead32(DartInfo[DartIndex].BaseAddress + DART_T8110_PARAMS4);
            DartInfo[DartIndex].Nsid = Params4 & DART_T8110_PARAMS4_NSID_MASK;
            DartInfo[DartIndex].Nttbr = 1;
            DartInfo[DartIndex].SidEnableBase = DART_T8110_SID_ENABLE_BASE;
            DartInfo[DartIndex].TcrBase = DART_T8110_TCR_BASE;
            DartInfo[DartIndex].TcrTranslateEnable = DART_T8110_TCR_TRANSLATE_ENABLE;
            DartInfo[DartIndex].TcrBypass = (DART_T8110_TCR_BYPASS_DAPF | DART_T8110_TCR_BYPASS_DART);
            DartInfo[DartIndex].TtbrBase = DART_T8110_TTBR_BASE;
            DartInfo[DartIndex].TtbrIsValid = DART_T8110_TTBR_VALID;
            DartInfo[DartIndex].BypassMode = FALSE; // Assume there's no bypass mode by default.
            DartInfo[DartIndex].TlbFlush = AppleDartT8110TlbFlush;

        }
        else {
            DEBUG((DEBUG_INFO, "%a - Setting up T8020-compatible DART\n", __FUNCTION__));
            DartInfo[DartIndex].Nsid = 16;
            DartInfo[DartIndex].Nttbr = 4;
            DartInfo[DartIndex].SidEnableBase = DART_T8020_SID_ENABLE;
            DartInfo[DartIndex].TcrBase = DART_T8020_TCR_BASE;
            DartInfo[DartIndex].TcrTranslateEnable = DART_T8020_TCR_TRANSLATE_ENABLE;
            DartInfo[DartIndex].TcrBypass = (DART_T8020_TCR_BYPASS_DAPF | DART_T8020_TCR_BYPASS_DART);
            DartInfo[DartIndex].TtbrBase = DART_T8020_TTBR_BASE;
            DartInfo[DartIndex].TtbrIsValid = DART_T8020_TTBR_VALID;
            DartInfo[DartIndex].BypassMode = FALSE; // Assume there's no bypass mode by default.
            DartInfo[DartIndex].TlbFlush = AppleDartT8020TlbFlush;
        }

        if((AsciiStrCmp(CompatibleStr ,"dart,t8110") == 0) || (AsciiStrCmp(CompatibleStr ,"dart,t6000") == 0)) {
            DartInfo[DartIndex].Shift = 4;
        }

        DartInfo[DartIndex].DmaVirtAddrBase = DART_PAGE_SIZE;
        DartInfo[DartIndex].DmaVirtAddrEnd = SIZE_4GB - DART_PAGE_SIZE;

        for(sid = 0; sid < DartInfo[DartIndex].Nsid; sid++) {
            MmioWrite32(DartInfo[DartIndex].BaseAddress + DART_TCR(DartInfo[DartIndex], sid), 0);
        }
        for(sid = 0; sid < DartInfo[DartIndex].Nsid; sid++) {
            for(i = 0; i < DartInfo[DartIndex].Nttbr; i++) {
                MmioWrite32(DartInfo[DartIndex].BaseAddress + DART_TTBR(DartInfo[DartIndex], sid, i), 0);
            }
        }

        DartInfo[DartIndex].TlbFlush((VOID *)&DartInfo[DartIndex]);

        Params2 = MmioRead32(DartInfo[DartIndex].BaseAddress + DART_PARAMS2);
        if((Params2 & DART_PARAMS2_BYPASS_SUPPORT) != 0) {
            DEBUG((DEBUG_INFO, "AppleDartIoMmuDxeInitialize: USB DART%d supports bypass mode\n", DartIndex));
            for(sid = 0; sid < DartInfo[DartIndex].Nsid; sid++) {
                MmioWrite32(DartInfo[DartIndex].BaseAddress + DART_TCR(DartInfo[DartIndex], sid), DartInfo[DartIndex].TcrBypass);
            }
            DartInfo[DartIndex].BypassMode = TRUE;
            //
            // Bypass mode means the controllers can just DMA right into physical memory so we skip installing the IOMMU protocol.
            //
        }

        //
        // if there's no bypass mode available for this DART, set up translation.
        // Warning: might not work for some devices given page size shenanigans
        // This code is disabled for now while I work out why USB-A DARTs aren't working.
        //
        // Ntte = DIV_ROUND_UP(DartInfo->DmaVirtAddrEnd, DART_PAGE_SIZE);
        // NL2 = DIV_ROUND_UP(Ntte, DART_PAGE_SIZE / sizeof(UINT64));
        // NL1 = DIV_ROUND_UP(NL2, DART_PAGE_SIZE / sizeof(UINT64));

        // DartInfo->L2 = AllocateAlignedPages(NL2 * DART_PAGE_SIZE, DART_PAGE_SIZE);
        // memset(DartInfo->L2, 0, NL2 * DART_PAGE_SIZE);
        // WriteBackInvalidateDataCacheRange((VOID *)DartInfo->L2, NL2 * DART_PAGE_SIZE);

        // DartInfo->L1 = AllocateAlignedPages(NL1 * DART_PAGE_SIZE, DART_PAGE_SIZE);
        // memset(DartInfo->L1, 0, NL1 * DART_PAGE_SIZE);
        // L2 = (PHYSICAL_ADDRESS)DartInfo->L2;
        // for(i = 0; i < NL2; i++) {
        //     DartInfo->L1[i] = (L2 >> DartInfo->Shift) | DART_L1_TABLE;
        //     L2 += DART_PAGE_SIZE;
        // }
        // WriteBackInvalidateDataCacheRange((VOID *)DartInfo->L1, NL1 * DART_PAGE_SIZE);

        // for(sid = 0; sid < DartInfo->Nsid; sid++) {
        //     Address = (PHYSICAL_ADDRESS)DartInfo->L1;
        //     for(i = 0; i < NL1; i++) {
        //         MmioWrite32(DartInfo->BaseAddress + DART_TTBR(DartInfo, sid, i), (Address >> DART_TTBR_SHIFT | DartInfo->TtbrIsValid));
        //         Address += DART_PAGE_SIZE;
        //     }
        // }
        // DartInfo->TlbFlush((VOID *)DartInfo);

        // for(i = 0; i < DartInfo->Nsid / 32; i++) {
        //     MmioWrite32(DartInfo->BaseAddress + DART_SID_ENABLE(DartInfo, i), ~0);
        // }

        // for(sid = 0; sid < DartInfo->Nsid; sid++) {
        //     MmioWrite32(DartInfo->BaseAddress + DART_TCR(DartInfo, sid), DartInfo->TcrTranslateEnable);
        // }

        // return gBS->InstallMultipleProtocolInterfaces (
        //                 &ImageHandle,
        //                 &gEdkiiIoMmuProtocolGuid,
        //                 &mAppleDartIoMmuProtocol,
        //                 NULL
        //                 );
    }
    DEBUG((DEBUG_INFO, "All done\n"));
    return EFI_SUCCESS;
}