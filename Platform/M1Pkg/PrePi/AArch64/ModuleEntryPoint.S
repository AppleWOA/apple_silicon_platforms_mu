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

ASM_FUNC(_ModuleEntryPoint)

  //code borrowed from OnePlus6TPkg/SurfaceDuoPkg/ArmPlatformPkg
  //verify we are in the right location in RAM
  adr x4, .
  ldr x5, =_ModuleEntryPoint
  cmp x4, x5
  //backup devicetree pointer into x20
  mov x20, x0
  b.ne _CopyFirmware
  b _ModuleEntrySetupStack

_CopyFirmware:
  //pull desired address, get offset from ModuleEntryPoint and desired FD base address
  ldr x6, =FixedPcdGet64(PcdFdBaseAddress)
  sub	x5, x5, x6 // x5 now holds offset of _ModuleEntryPoint from start of FD base
  sub	x4, x4, x5 // x4 now holds address of actual FD base

  //update return address as necessary
  sub	   x30, x30, x4
  add	   x30, x30, x6
  ldr	   x5, =FixedPcdGet64 (PcdFdSize)

// crappy memcpy borrowed from OnePlus6TPkg/SurfaceDuoPkg
_memcpy_loop:
  ldp	   x2, x3, [x4], #16
  stp	   x2, x3, [x6], #16
  subs   x5, x5, #16
  b.ne   _memcpy_loop

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

// if we get here, something *very bad* has happened.
_NeverReturn:
  b _NeverReturn