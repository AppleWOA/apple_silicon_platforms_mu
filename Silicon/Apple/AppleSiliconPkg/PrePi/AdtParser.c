/*
 * Copyright (c) 2015, Linaro Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 */

#include <Uefi.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugAgentLib.h>
#include <Library/DebugLib.h>
#include <Library/AppleDTLib.h>

BOOLEAN
EarlySetup (
  IN VOID     *BootArgsAddr,
  OUT UINT64  *SystemMemoryBase,
  OUT UINT64  *SystemMemorySize,
  IN    VOID  *PcdBootArgsDest,
  IN    VOID  *PcdAdtDest
  )
{

  struct boot_args *BootArgs = (struct boot_args *)BootArgsAddr;

  CopyMem(PcdBootArgsDest, BootArgsAddr, sizeof(struct boot_args));
  CopyMem(PcdAdtDest, (VOID*)BootArgs->devtree - BootArgs->virt_base + BootArgs->phys_base, BootArgs->devtree_size);

  if(dt_check(PcdAdtDest, BootArgs->devtree_size, NULL) != 0) {
    DEBUG((EFI_D_INFO | EFI_D_LOAD | EFI_D_ERROR, "no ADT supplied, exiting\n"));
    return FALSE;
  }

  *SystemMemoryBase = BootArgs->phys_base;
  *SystemMemorySize = BootArgs->phys_base + BootArgs->mem_size - BootArgs->phys_base;


  PatchPcdSet64(PcdFrameBufferAddress, BootArgs->video.base);
  PatchPcdSet64(PcdFrameBufferSize, BootArgs->video.stride * BootArgs->video.height);

  DEBUG((EFI_D_INFO | EFI_D_LOAD | EFI_D_ERROR, "Framebuffer address loaded to PCDs: 0x%llx (size 0x%llx)\n", PcdGet64(PcdFrameBufferAddress), PcdGet64(PcdFrameBufferSize)));

  return TRUE;
}
