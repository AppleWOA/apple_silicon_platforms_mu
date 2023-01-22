/** @file

  Patches BootMGFW to not attempt a read from or write to standard ARM PMC registers when running in EL2
  (TODO: redirect writes to a helper that will configure the equivalent Apple register)

  Copyright (c) 2022 DuoWoA authors (for original KernelErrataPatcher)
  Copyright (c) 2022-2023 amarioguy (for Apple changes)

  SPDX-License-Identifier: MIT

**/
#include "KernelErrataPatcher.h"

GCC_ASM_IMPORT (KernelErrataPatcherExitBootServices)
GCC_ASM_EXPORT (ExitBootServicesWrapper)

// Wrapper around Exit boot service for hooking into winload.efi

//EFI_STATUS
//EFIAPI
//ExitBootServicesWrapper (
//  IN EFI_HANDLE ImageHandle, IN UINTN MapKey
//  );
ExitBootServicesWrapper:
        // winload's OslFwpKernelSetupPhase1 seems to
        // always save the OsLoaderBlock parameter into
        // the x19 register right after stack frame setup
        mov x2, x19

        // some older implementations may also use x20
        mov x3, x20

        // some newer implementations may also use x24
        mov x4, x24

        // the x30 register contains the last return addr
        mov x5, x30

        // Go for it
        b KernelErrataPatcherExitBootServices


LoadImageWrapper:
        //patch bootmgr to redirect PMC accesses to the Apple regs
        b KernelErrataPatcherLoadImage