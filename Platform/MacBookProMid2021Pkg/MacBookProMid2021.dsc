## @file
#  MacBook Pro Mid 2021 DSC file, borrowing aspects from SurfaceDuo2.dsc from WOA-Project/SurfaceDuoPkg
#  Copyright (c) 2011-2015, ARM Limited. All rights reserved.
#  Copyright (c) 2014, Linaro Limited. All rights reserved.
#  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.
#  Copyright (c) 2018, Bingxing Wang. All rights reserved.
#
#  Modified copy of Platform/MacBookProEarly2023Pkg/MacBookProEarly2023.dsc
#
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################

[Defines]
  PLATFORM_NAME                  = MacBookProMid2021
  PLATFORM_GUID                  = 580c58c8-7ad6-11f0-a36f-eff621d5a815
  PLATFORM_VERSION               = 1.0
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/MacBookProMid2021-$(ARCH)
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = MacBookProMid2021Pkg/MacBookProMid2021.fdf
  SECURE_BOOT_ENABLE             = FALSE #disable secure boot for now
  AIC_BUILD                      = TRUE #AIC build enabled by default, change to false if you want to use a vGIC
  NETWORK_TLS_ENABLE             = TRUE

[BuildOptions.common]
  GCC:*_*_AARCH64_CC_FLAGS = -DSILICON_PLATFORM=6020
  *_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES -D HAS_MEMCPY_INTRINSICS



[PcdsFixedAtBuild.common]
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosSystemModel|"MacBook Pro (Early 2023)"
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosSystemModelNumber|"Mac14,5/Mac14,6/Mac14,9/Mac14,10"
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosSystemSku|"MacBook Pro (Early 2023) (Mac14,5/Mac14,6/Mac14,9/Mac14,10)"
  gAppleSiliconPkgTokenSpaceGuid.PcdSystemMemoryUefiRegionSize|0x8000000
  #will be changed later on, default values
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferWidth|3456
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferHeight|2234
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferPixelBpp|30

[PcdsPatchableInModule.common]

[PcdsDynamicDefault.common]
  #borrowed from SurfaceDuoPkg
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|3456
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|2234
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|3456
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|2234
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|300
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|50
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|300 
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|50

[Components.common]

  MacBookProMid2021Pkg/AcpiTables/DeviceAcpiTables.inf


!include T600XFamilyPkg/T600XFamilyPkg.dsc.inc
!include AppleSiliconPkg/AppleSiliconPkg.dsc.inc
!include AppleSiliconPkg/FrontpageDsc.inc