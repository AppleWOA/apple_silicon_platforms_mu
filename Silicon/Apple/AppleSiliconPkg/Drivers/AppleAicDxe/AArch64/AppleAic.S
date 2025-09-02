//AppleAicV2.S
//ASM helpers for the AIC DXE driver. (namely for FIQ sources)
//Copyright (c) amarioguy (Arminder Singh), 2022.
//SPDX-License-Identifier: BSD-2-clause-patent


#include <AsmMacroIoLibV8.h>
#include <Library/AppleSysRegs.h>

//UINTN EFIAPI AppleAicReadIpiStatusRegister(VOID);
ASM_FUNC(AppleAicReadIpiStatusRegister)
    mrs x0, APPLE_FAST_IPI_STATUS_REGISTER_EL1
    ret

//VOID EFIAPI AppleAicWriteIpiStatusRegister(IN UINTN Value);
ASM_FUNC(AppleAicWriteIpiStatusRegister)
    msr APPLE_FAST_IPI_STATUS_REGISTER_EL1, x0
    ret

//VOID EFIAPI AppleAicWriteIpiLocalRequestRegister(IN UINTN Value);
ASM_FUNC(AppleAicWriteIpiLocalRequestRegister)
    msr APPLE_FAST_IPI_REQUEST_LOCAL_EL1, x0
    ret

//VOID EFIAPI AppleAicWriteIpiGlobalRequestRegister(IN UINTN Value);
ASM_FUNC(AppleAicWriteIpiGlobalRequestRegister)
    msr APPLE_FAST_IPI_REQUEST_GLOBAL_EL1, x0
    ret

//UINTN EFIAPI AppleAicV2ReadPmcControlRegister(VOID);
ASM_FUNC(AppleAicReadPmcControlRegister)
    mrs x0, APPLE_PMCR0_EL1
    ret

//VOID EFIAPI AppleAicV2WritePmcControlRegister(IN UINTN Value);
ASM_FUNC(AppleAicWritePmcControlRegister)
    msr APPLE_PMCR0_EL1, x0
    ret

//UINTN EFIAPI AppleAicV2ReadUncorePmcControlRegister(VOID);
ASM_FUNC(AppleAicReadUncorePmcControlRegister)
    mrs x0, APPLE_UPMCR0_EL1
    ret

//VOID EFIAPI AppleAicV2WriteUncorePmcControlRegister(IN UINTN Value);
ASM_FUNC(AppleAicWriteUncorePmcControlRegister)
    msr APPLE_UPMCR0_EL1, x0
    ret

//UINTN EFIAPI AppleAicV2ReadUncorePmcStatusRegister(VOID)
ASM_FUNC(AppleAicReadUncorePmcStatusRegister)
    mrs x0, APPLE_UPMSR_EL1
    ret
