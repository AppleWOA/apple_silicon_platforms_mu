//AppleAicV2.S
//ASM helpers for the AIC DXE driver.
//Copyright (c) amarioguy (Arminder Singh), 2022.
//SPDX-License-Identifier: BSD-2-clause-patent


#include <AsmMacroIoLibV8.h>
#include <AppleSysRegs.h>

//UINTN EFIAPI AppleAicV2ReadIpiStatusRegister(VOID);
ASM_FUNC(AppleAicV2ReadIpiStatusRegister)
    mrs x0, APPLE_FAST_IPI_STATUS_REGISTER_EL1
    ret

//VOID EFIAPI AppleAicV2WriteIpiStatusRegister(IN UINTN Value);
ASM_FUNC(AppleAicV2WriteIpiStatusRegister)
    msr APPLE_FAST_IPI_STATUS_REGISTER_EL1, x0
    ret

//VOID EFIAPI AppleAicV2WriteIpiLocalRequestRegister(IN UINTN Value);
ASM_FUNC(AppleAicV2WriteIpiLocalRequestRegister)
    msr APPLE_FAST_IPI_REQUEST_LOCAL_EL1, x0
    ret

//VOID EFIAPI AppleAicV2WriteIpiGlobalRequestRegister(IN UINTN Value);
ASM_FUNC(AppleAicV2WriteIpiGlobalRequestRegister)
    msr APPLE_FAST_IPI_REQUEST_GLOBAL_EL1, x0
    ret

//UINTN EFIAPI AppleAicV2ReadPmcControlRegister(VOID);
ASM_FUNC(AppleAicV2ReadPmcControlRegister)
    mrs x0, APPLE_PMCR0_EL1
    ret

//VOID EFIAPI AppleAicV2WritePmcControlRegister(IN UINTN Value);
ASM_FUNC(AppleAicV2WritePmcControlRegister)
    msr APPLE_PMCR0_EL1, x0
    ret

//UINTN EFIAPI AppleAicV2ReadUncorePmcControlRegister(VOID);
ASM_FUNC(AppleAicV2ReadUncorePmcControlRegister)
    mrs x0, APPLE_UPMCR0_EL1
    ret

//VOID EFIAPI AppleAicV2WriteUncorePmcControlRegister(IN UINTN Value);
ASM_FUNC(AppleAicV2WriteUncorePmcControlRegister)
    msr APPLE_UPMCR0_EL1, x0
    ret

//UINTN EFIAPI AppleAicV2ReadUncorePmcStatusRegister(VOID)
ASM_FUNC(AppleAicV2ReadUncorePmcStatusRegister)
    mrs x0, APPLE_UPMSR_EL1
    ret