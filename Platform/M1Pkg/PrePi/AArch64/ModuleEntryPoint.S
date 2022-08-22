#include <Library/PcdLib.h>
#include <AsmMacroIoLibV8.h>
#include <Chipset/AArch64.h>
#include <Library/ArmLib.h>

.text
.align 3

GCC_ASM_IMPORT(CEntryPoint)
GCC_ASM_EXPORT(_ModuleEntryPoint)
GCC_ASM_IMPORT (ArmEnableInstructionCache)
GCC_ASM_IMPORT (ArmEnableDataCache)
GCC_ASM_IMPORT(InitializeUART)
GCC_ASM_IMPORT(UARTRelocationDebugMessage)

ASM_FUNC(_ModuleEntryPoint)

  //Use FDT to discover usable DRAM base/size
  bl ASM_PFX(DiscoverDramFromDt)
  bl ASM_PFX(InitializeUART)
  bl ASM_PFX(UARTRelocationDebugMessage)
  b _ModuleEntrySetupStack

_ModuleEntrySetupStack:

  // Compute Top of System Memory
  ldr   x1, PcdGet64 (PcdSystemMemoryBase)
  ldr   x2, PcdGet64 (PcdSystemMemorySize)
  sub   x2, x2, #1
  add   x1, x1, x2      // x1 = SystemMemoryTop = PcdSystemMemoryBase + PcdSystemMemorySize

  //calculate top of FD image
  ldr   x2, PcdGet64 (PcdFdBaseAddress)
  MOV64 (x3, FixedPcdGet32 (PcdFdSize) - 1)
  add   x3, x3, x2      // x3 = FdTop = PcdFdBaseAddress + PcdFdSize

  // UEFI Memory Size (stacks are allocated in this region)
  MOV32 (x4, FixedPcdGet32(PcdSystemMemoryUefiRegionSize))

  //
  // Reserve the memory for the UEFI region (contain stacks on its top)
  //

  // Calculate how much space there is between the top of the Firmware and the Top of the System Memory
  subs  x0, x1, x3   // x0 = SystemMemoryTop - FdTop
  b.mi  _SetupStack  // Jump if negative (FdTop > SystemMemoryTop). Case when the PrePi is in XIP memory outside of the DRAM
  cmp   x0, x4
  b.ge  _SetupStack

  // Case the top of stacks is the FdBaseAddress
  mov   x1, x2

_SetupStack:
  // x1 contains the top of the stack (and the UEFI Memory)

  // Because the 'push' instruction is equivalent to 'stmdb' (decrement before), we need to increment
  // one to the top of the stack. We check if incrementing one does not overflow (case of DRAM at the
  // top of the memory space)
  adds  x11, x1, #1
  b.cs  _SetupOverflowStack

_SetupAlignedStack:
  mov   x1, x11
  b     _GetBaseUefiMemory

_SetupOverflowStack:
  // Case memory at the top of the address space. Ensure the top of the stack is EFI_PAGE_SIZE
  // aligned (4KB)
  and   x1, x1, ~EFI_PAGE_MASK

_GetBaseUefiMemory:
  // Calculate the Base of the UEFI Memory
  sub   x11, x1, x4
  // backup EFI memory base pointer
  mov x22, x11

_GetStackBase:
  // Stack for the primary core = SecPhaseStack (base address in x12)
  //TODO - make the PCD value work properly, for now hardcode this value.
  MOV32 (x2, 0x20000)
  sub   x12, x1, x2

_StackSetup:
  //x1, top of stack
  //x12, stack base

  mov v4.d[0], xzr
  mov v4.d[1], xzr
  mov v5.2d, v4.2d 
  mov v6.2d, v4.2d
  mov v7.2d, v4.2d 
  
_ClearStack: 
  /* Assumes StackBase is 128-bit aligned, StackSize is a multiple of 64B */
  st4     {v4.2d, v5.2d, v6.2d, v7.2d}, [x12], #64  /* Fill every 64 bytes */
  cmp     x12, x2                                   /* Compare Size */ 
  b.lt     _ClearStack 

  //set stack pointer to addr in x1
  add sp, x1, xzr

_EnableCaches:
  bl ASM_PFX(ArmEnableInstructionCache)
  bl ASM_PFX(ArmEnableDataCache)

_PrepareArgumentsAndJump:
  mov x0, x12 //Stack Base
  mov x1, x2  //Stack Size
  mov x2, x20 //DeviceTree pointer
  mov x3, x22 //UEFI memory base

  bl CEntryPoint

// if we get here, something *very bad* has happened. (or we're debugging)
_NeverReturn:
  b _NeverReturn


// VOID
// DiscoverDramFromDt (
//   VOID   *DeviceTreeBaseAddress,   // passed by loader in x0
//   VOID   *ImageBase                // passed by FDF trampoline in x1
//   );
ASM_PFX(DiscoverDramFromDt):
  //
  // If we are booting from RAM using the Linux kernel boot protocol, x0 will
  // point to the DTB image in memory. Otherwise, use the default value defined
  // by the platform.
  //
  cbnz  x0, 0f
  ldr   x0, PcdGet64 (PcdFdtPointer)

0:mov   x28, x30            // preserve LR
  mov   x27, x0             // preserve DTB pointer
  mov   x26, x1             // preserve base of image pointer

  //
  // The base of the runtime image has been preserved in x1. Check whether
  // the expected magic number can be found in the header.
  //
  ldr   w8, .LArm64LinuxMagic
  ldr   w9, [x1, #0x38]
  cmp   w8, w9
  bne   .Lout

  //
  //
  // OK, so far so good. We have confirmed that we likely have a DTB and are
  // booting via the arm64 Linux boot protocol. Update the base-of-image PCD
  // to the actual relocated value, and add the shift of PcdFdBaseAddress to
  // PcdFvBaseAddress as well
  //
  adr   x8, PcdGet64 (PcdFdBaseAddress)
  adr   x9, PcdGet64 (PcdFvBaseAddress)
  ldr   x6, [x8]
  ldr   x7, [x9]
  sub   x7, x7, x6
  add   x7, x7, x1
  str   x1, [x8]
  str   x7, [x9]

  //
  // The runtime address may be different from the link time address so fix
  // up the PE/COFF relocations. Since we are calling a C function, use the
  // window at the beginning of the FD image as a temp stack.
  //
  mov   x0, x7
  adr   x1, PeCoffLoaderImageReadFromMemory
  mov   sp, x7
  bl    RelocatePeCoffImage

  //
  // Discover the memory size and offset from the DTB, and record in the
  // respective PCDs. This will also return false if a corrupt DTB is
  // encountered.
  //
  mov   x0, x27
  adr   x1, PcdGet64 (PcdSystemMemoryBase)
  adr   x2, PcdGet64 (PcdSystemMemorySize)
  bl    FindMemnode
  cbz   x0, .Lout

  //
  // Copy the DTB to the slack space right after the 64 byte arm64/Linux style
  // image header at the base of this image (defined in the FDF), and record the
  // pointer in PcdFdtPointer.
  //
  adr   x8, PcdGet64 (PcdFdtPointer)
  add   x26, x26, #0x40
  str   x26, [x8]

  mov   x0, x26
  mov   x1, x27
  bl    CopyFdt

.Lout:
  ret    x28

.LArm64LinuxMagic:
  .byte   0x41, 0x52, 0x4d, 0x64
