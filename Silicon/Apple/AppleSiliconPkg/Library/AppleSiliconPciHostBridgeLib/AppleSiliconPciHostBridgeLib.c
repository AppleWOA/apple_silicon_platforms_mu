/**
 * Copyright (c) 2023, amarioguy (AppleWOA authors).
 * 
 * Module Name:
 *     AppleSiliconPciHostBridgeLib.c
 * 
 * Abstract:
 *     PCI Host Bridge Driver library for Apple Silicon platforms.
 *     Based on SbsaQemuPciHostBridgeLib.
 * 
 * Environment:
 *     UEFI DXE (Driver Execution Environment).
 * 
 * License:
 *     SPDX-License-Identifier: BSD-2-Clause-Patent OR MIT
 * 
*/

#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PciHostBridgeLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <PiDxe.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/PciHostBridgeResourceAllocation.h>

#pragma pack(1)
typedef struct {
  ACPI_HID_DEVICE_PATH        AcpiDevicePath;
  EFI_DEVICE_PATH_PROTOCOL    EndDevicePath;
} EFI_PCI_ROOT_BRIDGE_DEVICE_PATH;
#pragma pack ()

STATIC EFI_PCI_ROOT_BRIDGE_DEVICE_PATH  mEfiPciRootBridgeDevicePath = {
  {
    {
      ACPI_DEVICE_PATH,
      ACPI_DP,
      {
        (UINT8)(sizeof (ACPI_HID_DEVICE_PATH)),
        (UINT8)((sizeof (ACPI_HID_DEVICE_PATH)) >> 8)
      }
    },
    EISA_PNP_ID (0x0A03),
    0
  },

  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      END_DEVICE_PATH_LENGTH,
      0
    }
  }
};



STATIC PCI_ROOT_BRIDGE  mRootBridge = {
  /* UINT32 Segment; Segment number */
  0,

  /* UINT64 Supports; Supported attributes */
  0,

  /* UINT64 Attributes; Initial attributes */
  0,

  /* BOOLEAN DmaAbove4G; DMA above 4GB memory */
  TRUE,

  /* BOOLEAN NoExtendedConfigSpace; When FALSE, the root bridge supports
     Extended (4096-byte) Configuration Space.  When TRUE, the root bridge
     supports 256-byte Configuration Space only. */
  FALSE,

  /* BOOLEAN ResourceAssigned; Resource assignment status of the root bridge.
     Set to TRUE if Bus/IO/MMIO resources for root bridge have been assigned */
  FALSE,

  /* UINT64 AllocationAttributes; Allocation attributes. */
  EFI_PCI_HOST_BRIDGE_COMBINE_MEM_PMEM |
  EFI_PCI_HOST_BRIDGE_MEM64_DECODE,                                         /* as Mmio64Size > 0 */

  {
    /* PCI_ROOT_BRIDGE_APERTURE Bus; Bus aperture which can be used by the
     * root bridge. */
    FixedPcdGet32 (PcdPciBusMin),
    FixedPcdGet32 (PcdPciBusMax)
  },

  /* PCI_ROOT_BRIDGE_APERTURE Io; IO aperture which can be used by the root
     bridge. */
  // {
  //   FixedPcdGet64 (PcdPciIoBase),
  //   FixedPcdGet64 (PcdPciIoBase) + FixedPcdGet64 (PcdPciIoSize) - 1
  // },
  { MAX_UINT64,                                                             0  },
  /* PCI_ROOT_BRIDGE_APERTURE Mem; MMIO aperture below 4GB which can be used by
     the root bridge
     (gEfiMdePkgTokenSpaceGuid.PcdPciMmio32Translation as 0x0) */
  {
    FixedPcdGet32 (PcdPciMmio32Base),
    FixedPcdGet32 (PcdPciMmio32Base) + FixedPcdGet32 (PcdPciMmio32Size) - 1,
  },

  /* PCI_ROOT_BRIDGE_APERTURE MemAbove4G; MMIO aperture above 4GB which can be
     used by the root bridge.
     (gEfiMdePkgTokenSpaceGuid.PcdPciMmio64Translation as 0x0) */
  {
    FixedPcdGet64 (PcdPciMmio64Base),
    FixedPcdGet64 (PcdPciMmio64Base) + FixedPcdGet64 (PcdPciMmio64Size) - 1
  },

  /* PCI_ROOT_BRIDGE_APERTURE PMem; Prefetchable MMIO aperture below 4GB which
     can be used by the root bridge.
     
     For Apple chips, the 32-bit mapping is not prefetchable.
      */
  { MAX_UINT64,                                                             0  },

  /* PCI_ROOT_BRIDGE_APERTURE PMemAbove4G; Prefetchable MMIO aperture above 4GB
     which can be used by the root bridge. */
  { MAX_UINT64,                                                             0  },
  /* EFI_DEVICE_PATH_PROTOCOL *DevicePath; Device path. */
  (EFI_DEVICE_PATH_PROTOCOL *)&mEfiPciRootBridgeDevicePath,
};

GLOBAL_REMOVE_IF_UNREFERENCED
CHAR16  *mPciHostBridgeLibAcpiAddressSpaceTypeStr[] = {
  L"Mem", L"I/O", L"Bus"
};

//
// Description:
//   Return the pointer to the PCI root bridge descriptor array.
//
// Parameters:
//   Count - returns the count of root bridges on the system.
//
// Return value:
//   None.
//
PCI_ROOT_BRIDGE * EFIAPI PciHostBridgeGetRootBridges(UINTN *Count) {
  *Count = 1;
  return &mRootBridge;
}

//
// Description:
//   Frees the root bridge array.
//   Currently a no-op.
//
// Return value:
//   None.
//
VOID EFIAPI PciHostBridgeFreeRootBridges(PCI_ROOT_BRIDGE *Bridges, UINTN Count) {
  //
  // For now this is a no-op.
  //
  ASSERT(Count == 1);
  return;
}


VOID EFIAPI PciHostBridgeResourceConflict(EFI_HANDLE HostBridgeHandle, VOID *Configuration) {
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR  *Descriptor;
  UINTN                              RootBridgeIndex;

  DEBUG ((DEBUG_ERROR, "PciHostBridge: Resource conflict happens!\n"));

  RootBridgeIndex = 0;
  Descriptor      = (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *)Configuration;
  while (Descriptor->Desc == ACPI_ADDRESS_SPACE_DESCRIPTOR) {
    DEBUG ((DEBUG_ERROR, "RootBridge[%d]:\n", RootBridgeIndex++));
    for ( ; Descriptor->Desc == ACPI_ADDRESS_SPACE_DESCRIPTOR; Descriptor++) {
      ASSERT (
        Descriptor->ResType <
        ARRAY_SIZE (mPciHostBridgeLibAcpiAddressSpaceTypeStr)
        );
      DEBUG ((
        DEBUG_ERROR,
        " %s: Length/Alignment = 0x%lx / 0x%lx\n",
        mPciHostBridgeLibAcpiAddressSpaceTypeStr[Descriptor->ResType],
        Descriptor->AddrLen,
        Descriptor->AddrRangeMax
        ));
      if (Descriptor->ResType == ACPI_ADDRESS_SPACE_TYPE_MEM) {
        DEBUG ((
          DEBUG_ERROR,
          "     Granularity/SpecificFlag = %ld / %02x%s\n",
          Descriptor->AddrSpaceGranularity,
          Descriptor->SpecificFlag,
          ((Descriptor->SpecificFlag &
            EFI_ACPI_MEMORY_RESOURCE_SPECIFIC_FLAG_CACHEABLE_PREFETCHABLE
            ) != 0) ? L" (Prefetchable)" : L""
          ));
      }
    }

    //
    // Skip the END descriptor for root bridge
    //
    ASSERT (Descriptor->Desc == ACPI_END_TAG_DESCRIPTOR);
    Descriptor = (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *)(
                                                       (EFI_ACPI_END_TAG_DESCRIPTOR *)Descriptor + 1
                                                       );
  }
}