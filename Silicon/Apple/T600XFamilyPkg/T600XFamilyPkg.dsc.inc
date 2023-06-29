# Mac Studio family DSC include file
#  Copyright (c) 2011-2015, ARM Limited. All rights reserved.
#  Copyright (c) 2014, Linaro Limited. All rights reserved.
#  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.
#  Copyright (c) 2018 - 2019, Bingxing Wang. All rights reserved.
#
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
#
# SPDX-License-Identifier: BSD-2-Clause


[PcdsPatchableInModule.common]
  #These will get overriden by the FDT settings
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x10000000000
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x400000000 #16GB RAM space by default
  gArmTokenSpaceGuid.PcdFdBaseAddress|0x0
  gArmTokenSpaceGuid.PcdFvBaseAddress|0x0
  gAppleSiliconPkgTokenSpaceGuid.PcdFdtPointer|0x10A00000000
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferAddress|0x0
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferSize|0x0

[PcdsFixedAtBuild.common]
  #ARM architectural fixed build-time settings
  gArmTokenSpaceGuid.PcdArmArchTimerFreqInHz|24000000
  gArmTokenSpaceGuid.PcdCpuVectorBaseAddress|0x100FF000000 #fair warning this location is *totally random*
  gArmTokenSpaceGuid.PcdArmArchTimerSecIntrNum|19 # placeholder 
  gArmTokenSpaceGuid.PcdArmArchTimerIntrNum|17 # phys timer IRQ number (this is arbitrary because per-cpu fiq)
  gArmTokenSpaceGuid.PcdArmArchTimerHypIntrNum|20 #this disables the HV timer
  gArmTokenSpaceGuid.PcdArmArchTimerVirtIntrNum|18 #virt timer
  gAppleSiliconPkgTokenSpaceGuid.PcdAppleUartBase|0x39b200000
  gAppleSiliconPkgTokenSpaceGuid.PcdAppleSocIdentifier|0x6000
  gAppleSiliconPkgTokenSpaceGuid.PcdInitializeRamdisk|TRUE
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosCpuIdentifier|"T6000"
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosCpuModel|"Apple M1 Pro/Max/Ultra"
  gAppleSiliconPkgTokenSpaceGuid.PcdSecPhaseStackBase|0x10300000000
#AIC/GIC configuration settings
!if $(AIC_BUILD) == FALSE
  #On M1v2, this is a lot easier to pick as most of the 40 bit address space is reserved for MMIO and a lot goes unused
  gArmTokenSpaceGuid.PcdGicDistributorBase|0x5000000000
  gArmTokenSpaceGuid.PcdGicRedistributorsBase|0x5100000000
!else
  #this is MMIO space where the AIC lives
  gAppleSiliconPkgTokenSpaceGuid.PcdAicInterruptControllerBase|0x28e100000

!endif
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemRevision|0x00006000

  #
  # Make VariableRuntimeDxe work at emulated non-volatile variable mode.
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdEmuVariableNvModeEnable|TRUE

!if $(SECURE_BOOT_ENABLE) == TRUE
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesData|600
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesCode|400
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiBootServicesCode|1500
!else
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesData|300
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesCode|150
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiBootServicesCode|1000
!endif
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiBootServicesData|800
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiLoaderCode|10
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiLoaderData|0
  gEmbeddedTokenSpaceGuid.PcdPrePiCpuIoSize|40
  
  #PCIe settings
  gEfiMdeModulePkgTokenSpaceGuid.PcdSrIovSupport|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdAriSupport|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdMrIovSupport|FALSE


[LibraryClasses.common]
  MemoryInitPeiLib|T600XFamilyPkg/Library/MemoryInitPeiLib/MemoryInitPeiLib.inf
  MsPlatformDevicesLib  |T600XFamilyPkg/Library/MsPlatformDevicesLib/MsPlatformDevicesLib.inf

[Components.common]

  T600XFamilyPkg/AcpiTables/CPUAcpiTables.inf
