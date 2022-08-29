/*
 * Copyright (c) 2015, Linaro Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 */

#include <Uefi.h>
#include <Include/libfdt.h>
#include <Library/DebugAgentLib.h>
#include <Library/DebugLib.h>

BOOLEAN
FindMemnode (
  IN  VOID    *DeviceTreeBlob,
  OUT UINT64  *SystemMemoryBase,
  OUT UINT64  *SystemMemorySize
  )
{
  INT32        MemoryNode;
  INT32        AddressCells;
  INT32        SizeCells;
  INT32        Length;
  CONST INT32  *Prop;

  if (fdt_check_header (DeviceTreeBlob) != 0) {
    DEBUG((EFI_D_INFO | EFI_D_LOAD | EFI_D_ERROR, "no FDT supplied, exiting\n"));
    return FALSE;
  }

  //
  // Look for a node called "memory" at the lowest level of the tree
  //
  DEBUG((EFI_D_INFO | EFI_D_LOAD | EFI_D_ERROR, "looking for /memory node in FDT @ %p\n", DeviceTreeBlob));
  MemoryNode = fdt_path_offset (DeviceTreeBlob, "/memory");
  if (MemoryNode <= 0) {
    DEBUG((EFI_D_INFO | EFI_D_LOAD | EFI_D_ERROR, "cannot find /memory node in FDT, exiting\n"));
    return FALSE;
  }

  //
  // Retrieve the #address-cells and #size-cells properties
  // from the root node, or use the default if not provided.
  //
  AddressCells = 1;
  SizeCells    = 1;

  DEBUG((EFI_D_INFO | EFI_D_LOAD | EFI_D_ERROR, "looking for address-cells property in FDT\n"));
  Prop = fdt_getprop (DeviceTreeBlob, 0, "#address-cells", &Length);
  if (Length == 4) {
    DEBUG((EFI_D_INFO | EFI_D_LOAD | EFI_D_ERROR, "address-cells property = %p\n", Prop));
    AddressCells = fdt32_to_cpu (*Prop);
    DEBUG((EFI_D_INFO | EFI_D_LOAD | EFI_D_ERROR, "address-cells = 0x%lx\n", AddressCells));
  }

  DEBUG((EFI_D_INFO | EFI_D_LOAD | EFI_D_ERROR, "looking for size-cells property in FDT\n"));
  Prop = fdt_getprop (DeviceTreeBlob, 0, "#size-cells", &Length);
  if (Length == 4) {
    DEBUG((EFI_D_INFO | EFI_D_LOAD | EFI_D_ERROR, "size-cells property = %p\n", Prop));
    SizeCells = fdt32_to_cpu (*Prop);
    DEBUG((EFI_D_INFO | EFI_D_LOAD | EFI_D_ERROR, "size-cells = 0x%lx\n", AddressCells));
  }

  //
  // Now find the 'reg' property of the /memory node, and read the first
  // range listed.
  //
  DEBUG((EFI_D_INFO | EFI_D_LOAD | EFI_D_ERROR, "searching for reg property in /memory node\n"));
  Prop = fdt_getprop (DeviceTreeBlob, MemoryNode, "reg", &Length);
  DEBUG((EFI_D_INFO | EFI_D_LOAD | EFI_D_ERROR, "/memory/reg property = %p\n"));
  if (Length < (AddressCells + SizeCells) * sizeof (INT32)) {
    DEBUG((EFI_D_INFO | EFI_D_LOAD | EFI_D_ERROR, "invalid or missing /memory/reg property, exiting\n"));
    return FALSE;
  }

  *SystemMemoryBase = fdt32_to_cpu (Prop[0]);
  if (AddressCells > 1) {
    *SystemMemoryBase = (*SystemMemoryBase << 32) | fdt32_to_cpu (Prop[1]);
  }

  DEBUG((EFI_D_INFO | EFI_D_LOAD | EFI_D_ERROR, "New System RAM Base = %p\n", SystemMemoryBase));

  Prop += AddressCells;

  *SystemMemorySize = fdt32_to_cpu (Prop[0]);
  if (SizeCells > 1) {
    *SystemMemorySize = (*SystemMemorySize << 32) | fdt32_to_cpu (Prop[1]);
  }
  DEBUG((EFI_D_INFO | EFI_D_LOAD | EFI_D_ERROR, "New System RAM Size = %p\n", SystemMemorySize));

  return TRUE;
}

VOID
CopyFdt (
  IN    VOID  *FdtDest,
  IN    VOID  *FdtSource
  )
{
  fdt_pack (FdtSource);
  DEBUG((EFI_D_INFO | EFI_D_LOAD | EFI_D_ERROR, "Copying FDT from %p to %p\n", FdtSource, FdtDest));
  CopyMem (FdtDest, FdtSource, fdt_totalsize (FdtSource));
}
