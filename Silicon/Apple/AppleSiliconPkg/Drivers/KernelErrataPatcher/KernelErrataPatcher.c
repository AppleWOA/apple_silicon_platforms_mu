/** @file

  Based on SurfaceDuoPkg KernelErrataPatcher.

  Patches NTOSKRNL to change reads to PMCCNTR_EL0 to instead read from the Apple-specific variant of the register.
  
  Patches NTOSKRNL to not cause a bugcheck when attempting to use
  PSCI_MEMPROTECT Due to an issue in QHEE

  Based on https://github.com/SamuelTulach/rainbow

  Copyright (c) 2021 Samuel Tulach
  Copyright (c) 2022-2023 DuoWoA authors

  AppleWOA specific changes, Copyright (c) 2023, AppleWOA authors

  SPDX-License-Identifier: MIT

**/
#include "KernelErrataPatcher.h"

STATIC EFI_EXIT_BOOT_SERVICES EfiExitBootServices = NULL;

// Please see ./ShellCode/Reference/ShellCode.c for what this does
UINT8 OslArm64TransferToKernelShellCode[] = {
    0xEE, 0x03, 0x00, 0xAA, 0xC8, 0x0D, 0x41, 0xF8, 0x1F, 0x01, 0x0E, 0xEB,
    0xA0, 0x08, 0x00, 0x54, 0x07, 0x05, 0x82, 0x52, 0x07, 0xA7, 0xBA, 0x72,
    0x06, 0x55, 0x9A, 0x52, 0x66, 0xA7, 0xBA, 0x72, 0x0A, 0xFB, 0x9D, 0x52,
    0xEA, 0x5C, 0xA5, 0x72, 0xAB, 0x5A, 0x80, 0xD2, 0x0B, 0x00, 0xA3, 0xF2,
    0x6B, 0x00, 0xC0, 0xF2, 0x0B, 0x50, 0xFA, 0xF2, 0x6D, 0x00, 0x80, 0xD2,
    0x0D, 0x50, 0xBA, 0xF2, 0x4D, 0x00, 0xC0, 0xF2, 0x0D, 0x50, 0xFA, 0xF2,
    0x30, 0x00, 0x80, 0xD2, 0x10, 0x50, 0xBA, 0xF2, 0x10, 0x48, 0xC0, 0xF2,
    0x10, 0x00, 0xE3, 0xF2, 0x11, 0x78, 0x80, 0x52, 0xF1, 0xCB, 0xBA, 0x72,
    0x4F, 0x00, 0x80, 0xD2, 0x0F, 0x50, 0xBA, 0xF2, 0x2F, 0x00, 0xC0, 0xF2,
    0x0F, 0x50, 0xFA, 0xF2, 0xEC, 0x03, 0x84, 0x52, 0x6C, 0xA0, 0xBA, 0x72,
    0x09, 0x01, 0x80, 0x52, 0x09, 0x50, 0xBA, 0x72, 0x20, 0x00, 0x00, 0x14,
    0x63, 0x00, 0x0A, 0x0B, 0x7F, 0x04, 0x00, 0x71, 0x69, 0x01, 0x00, 0x54,
    0x43, 0x00, 0x40, 0xF9, 0x7F, 0x00, 0x0B, 0xEB, 0x40, 0x02, 0x00, 0x54,
    0x7F, 0x00, 0x0D, 0xEB, 0xE1, 0x00, 0x00, 0x54, 0x43, 0x04, 0x40, 0xF9,
    0x7F, 0x00, 0x10, 0xEB, 0x81, 0x00, 0x00, 0x54, 0x51, 0x40, 0x1E, 0xB8,
    0x02, 0x00, 0x00, 0x14, 0x4C, 0x00, 0x00, 0xB9, 0x42, 0x10, 0x00, 0x91,
    0x5F, 0x00, 0x05, 0xEB, 0x82, 0x01, 0x00, 0x54, 0x43, 0x00, 0x40, 0xB9,
    0x7F, 0x00, 0x07, 0x6B, 0x64, 0x10, 0x46, 0x7A, 0x81, 0xFD, 0xFF, 0x54,
    0x49, 0x00, 0x00, 0xB9, 0xF8, 0xFF, 0xFF, 0x17, 0x43, 0x04, 0x40, 0xF9,
    0x7F, 0x00, 0x0F, 0xEB, 0xA1, 0xFE, 0xFF, 0x54, 0x51, 0x00, 0x1E, 0xB8,
    0xF3, 0xFF, 0xFF, 0x17, 0x08, 0x01, 0x40, 0xF9, 0x1F, 0x01, 0x0E, 0xEB,
    0x20, 0x01, 0x00, 0x54, 0x02, 0x19, 0x40, 0xF9, 0x05, 0x41, 0x40, 0xB9,
    0xA5, 0x00, 0x02, 0x8B, 0x5F, 0x00, 0x05, 0xEB, 0xC3, 0xFD, 0xFF, 0x54,
    0xF8, 0xFF, 0xFF, 0x17, 0x00, 0x00, 0x00, 0x14, 0x1F, 0x20, 0x03, 0xD5};

#if LEGACY == 1
VOID PatchKernelComponents(PLOADER_PARAMETER_BLOCK loaderBlock)
{
  for (LIST_ENTRY *entry = (&loaderBlock->LoadOrderListHead)->ForwardLink;
       entry != (&loaderBlock->LoadOrderListHead); entry = entry->ForwardLink) {

    PKLDR_DATA_TABLE_ENTRY kernelModule =
        CONTAINING_RECORD(entry, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

    EFI_PHYSICAL_ADDRESS base = (EFI_PHYSICAL_ADDRESS)kernelModule->DllBase;
    UINTN                size = kernelModule->SizeOfImage;

    for (EFI_PHYSICAL_ADDRESS current = base; current < base + size;
         current += sizeof(UINT32)) {
      if (*(UINT32 *)current == 0xD5381028 || // mrs x8, actlr_el1
          *(UINT32 *)current == 0xD53BD2A8) { // mrs x8,
                                              // amcntenset0_el0
        *(UINT32 *)current = 0xD2800008;      // movz x8, #0
      }
      else if (
          *(UINT32 *)current == 0xD5181028 || // msr actlr_el1, x8
          *(UINT32 *)current == 0xD5181029) { // msr actlr_el1, x9
        *(UINT32 *)current = 0xD503201F;      // nop
      }
      else if (
          *(UINT64 *)current == 0xD2800003180002D5 &&
          *(UINT64 *)(current + ARM64_TOTAL_INSTRUCTION_LENGTH(2)) ==
              0xD2800001D2800002) { // ldr w21, #0x58 - movz x3, #0 - movz x2,
                                    // #0 - movz x1, #0
        *(UINT32 *)(current - ARM64_TOTAL_INSTRUCTION_LENGTH(8)) =
            0xD65F03C0; // ret
      }
      else if (
          *(UINT64 *)current == 0xD2800002D2800003 &&
          *(UINT64 *)(current + ARM64_TOTAL_INSTRUCTION_LENGTH(2)) ==
              0x18000240D2800001) { // movz x3, #0 - movz x2, #0 - movz x1, #0 -
                                    // ldr w0, #0x54
        *(UINT32 *)(current - ARM64_TOTAL_INSTRUCTION_LENGTH(7)) =
            0xD65F03C0; // ret
      }
    }
  }
}
#endif

VOID KernelErrataPatcherApplyReadACTLREL1Patches(
    EFI_PHYSICAL_ADDRESS Base, UINTN Size)
{
  // Fix up #0 (28 10 38 D5 -> 08 00 80 D2) (ACTRL_EL1 Register Read)
  UINT8                FixedInstruction0[] = {0x08, 0x00, 0x80, 0xD2};
  EFI_PHYSICAL_ADDRESS IllegalInstruction0 =
      FindPattern(Base, Size, "28 10 38 D5");

  if (IllegalInstruction0 != 0) {
    FirmwarePrint("mrs x8, actlr_el1         -> 0x%p\n", IllegalInstruction0);

    CopyMemory(
        IllegalInstruction0, (EFI_PHYSICAL_ADDRESS)FixedInstruction0,
        sizeof(FixedInstruction0));
  }
}

EFI_STATUS
EFIAPI
KernelErrataPatcherExitBootServices(
    IN EFI_HANDLE ImageHandle, IN UINTN MapKey,
#if LEGACY == 1
    IN PLOADER_PARAMETER_BLOCK loaderBlockX19,
    IN PLOADER_PARAMETER_BLOCK loaderBlockX20,
    IN PLOADER_PARAMETER_BLOCK loaderBlockX24,
#endif
    IN EFI_PHYSICAL_ADDRESS fwpKernelSetupPhase1)
{
  // Might be called multiple times by winload in a loop failing few times
  gBS->ExitBootServices = EfiExitBootServices;

  FirmwarePrint(
      "OslFwpKernelSetupPhase1   -> (phys) 0x%p\n", fwpKernelSetupPhase1);

  UINTN                tempSize = SCAN_MAX;
  EFI_PHYSICAL_ADDRESS winloadBase =
      LocateWinloadBase(fwpKernelSetupPhase1, &tempSize);

  EFI_STATUS Status = UnprotectWinload(winloadBase + EFI_PAGE_SIZE, tempSize);
  if (EFI_ERROR(Status)) {
    FirmwarePrint(
        "Could not unprotect winload -> (phys) 0x%p (size) 0x%p %r\n",
        winloadBase, tempSize, Status);

    goto exit;
  }

  // Fix up winload.efi
  // This fixes Boot Debugger
  FirmwarePrint(
      "Patching OsLoader         -> (phys) 0x%p (size) 0x%p\n",
      fwpKernelSetupPhase1, SCAN_MAX);

  KernelErrataPatcherApplyReadACTLREL1Patches(fwpKernelSetupPhase1, SCAN_MAX);

  EFI_PHYSICAL_ADDRESS OslArm64TransferToKernelAddr =
      winloadBase + 0xC00 +
      NT_OS_LOADER_ARM64_TRANSFER_TO_KERNEL_FUNCTION_OFFSET;
  EFI_PHYSICAL_ADDRESS NewOslArm64TransferToKernelAddr =
      OslArm64TransferToKernelAddr - sizeof(OslArm64TransferToKernelShellCode);

  for (EFI_PHYSICAL_ADDRESS current = OslArm64TransferToKernelAddr;
       current < OslArm64TransferToKernelAddr + SCAN_MAX;
       current += sizeof(UINT32)) {

    if (ARM64_BRANCH_LOCATION_INSTRUCTION(
            current, OslArm64TransferToKernelAddr) == *(UINT32 *)current) {
      FirmwarePrint(
          "Patching bl OsLoaderArm64TransferToKernel -> (phys) 0x%p\n",
          current);

      *(UINT32 *)current = ARM64_BRANCH_LOCATION_INSTRUCTION(
          current, NewOslArm64TransferToKernelAddr);

      FirmwarePrint(
          "Patching OsLoaderArm64TransferToKernel -> (phys) 0x%p\n",
          OslArm64TransferToKernelAddr);

      // Copy shell code right before the OsLoaderArm64TransferToKernelFunction
      CopyMemory(
          NewOslArm64TransferToKernelAddr,
          (EFI_PHYSICAL_ADDRESS)OslArm64TransferToKernelShellCode,
          sizeof(OslArm64TransferToKernelShellCode));

      break;
    }
  }

  Status = ReProtectWinload(winloadBase + EFI_PAGE_SIZE, tempSize);
  if (EFI_ERROR(Status)) {
    FirmwarePrint(
        "Could not reprotect winload -> (phys) 0x%p (size) 0x%p %r\n",
        winloadBase, tempSize, Status);

    goto exit;
  }

#if LEGACY == 1
  PLOADER_PARAMETER_BLOCK loaderBlock = loaderBlockX19;

  if (loaderBlock == NULL ||
      ((EFI_PHYSICAL_ADDRESS)loaderBlock & 0xFFFFFFF000000000) == 0) {
    loaderBlock = loaderBlockX20;
  }

  if (loaderBlock == NULL ||
      ((EFI_PHYSICAL_ADDRESS)loaderBlock & 0xFFFFFFF000000000) == 0) {
    loaderBlock = loaderBlockX24;
  }

  if (loaderBlock == NULL ||
      ((EFI_PHYSICAL_ADDRESS)loaderBlock & 0xFFFFFFF000000000) == 0) {
    goto exit;
  }

  EFI_PHYSICAL_ADDRESS PatternMatch = FindPattern(
      fwpKernelSetupPhase1, SCAN_MAX,
      "1F 04 00 71 33 11 88 9A 28 00 40 B9 1F 01 00 6B");

  BL_ARCH_SWITCH_CONTEXT BlpArchSwitchContext =
      (BL_ARCH_SWITCH_CONTEXT)(PatternMatch -
                               ARM64_TOTAL_INSTRUCTION_LENGTH(9));

  // First check if the version of BlpArchSwitchContext before Memory Management
  // v2 is found
  if (PatternMatch == 0 || (PatternMatch & 0xFFFFFFF000000000) != 0) {
    // Okay, we maybe have the new Memory Management? Try again.
    PatternMatch =
        FindPattern(fwpKernelSetupPhase1, SCAN_MAX, "9F 06 00 71 33 11 88 9A");

    BlpArchSwitchContext =
        (BL_ARCH_SWITCH_CONTEXT)(PatternMatch -
                                 ARM64_TOTAL_INSTRUCTION_LENGTH(24));

    if (PatternMatch == 0 || (PatternMatch & 0xFFFFFFF000000000) != 0) {
      // Okay, we maybe have the new new Memory Management? Try again.
      PatternMatch = FindPattern(
          fwpKernelSetupPhase1, SCAN_MAX, "7F 06 00 71 37 11 88 9A");

      BlpArchSwitchContext =
          (BL_ARCH_SWITCH_CONTEXT)(PatternMatch -
                                   ARM64_TOTAL_INSTRUCTION_LENGTH(24));

      if (PatternMatch == 0 || (PatternMatch & 0xFFFFFFF000000000) != 0) {
        goto exit;
      }
    }
  }

  /*
   * Switch context to (as defined by winload) application context
   * Within this context only the virtual addresses are valid
   * Real/physical addressing is not used
   * We can not use any EFI services unless we switch back!
   * To print on screen use ContextPrint define
   */
  BlpArchSwitchContext(ApplicationContext);

  UINT32 OsMajorVersion = loaderBlock->OsMajorVersion;
  UINT32 OsMinorVersion = loaderBlock->OsMinorVersion;
  UINT32 Size           = loaderBlock->Size;

  if (OsMajorVersion != 10 || OsMinorVersion != 0 || Size == 0) {
    goto exitToFirmware;
  }

  PatchKernelComponents(loaderBlock);

exitToFirmware:
  // Switch back to firmware context before calling real ExitBootServices
  BlpArchSwitchContext(FirmwareContext);
#endif

exit:
  FirmwarePrint(
      "OslFwpKernelSetupPhase1   <- (phys) 0x%p\n", fwpKernelSetupPhase1);

  // Call the original
  return gBS->ExitBootServices(ImageHandle, MapKey);
}

EFI_STATUS
EFIAPI
KernelErrataPatcherEntryPoint(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EfiExitBootServices   = gBS->ExitBootServices;
  gBS->ExitBootServices = ExitBootServicesWrapper;

  return InitMemoryAttributeProtocol();
}