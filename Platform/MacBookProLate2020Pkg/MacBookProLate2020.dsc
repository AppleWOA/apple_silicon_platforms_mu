#Mac Mini 2020 UEFI DSC file
#some parts borrowed from WOA-Project/SurfaceDuoPkg
#Disclaimer: probably not the best UEFI dev out there
#
#  Copyright (c) 2011-2015, ARM Limited. All rights reserved.
#  Copyright (c) 2014, Linaro Limited. All rights reserved.
#  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.
#  Copyright (c) 2018, Bingxing Wang. All rights reserved.
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

#SPDX-License-Identifier: BSD 2-Clause

#Basic Defines

[Defines]
  PLATFORM_NAME                  = MacBookProLate2020
  PLATFORM_GUID                  = 1d0c231c-6c6f-4cbf-b147-da738f745bca
  PLATFORM_VERSION               = 1.0
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/MacBookProLate2020-$(ARCH)
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = MacBookProLate2020/MacBookProLate2020.fdf
  SECURE_BOOT_ENABLE             = FALSE #disable secure boot for now
  AIC_BUILD                      = TRUE #AIC build enabled by default, change to false if you want to use a vGIC
  NETWORK_TLS_ENABLE             = TRUE


[BuildOptions.common]
  GCC:*_*_AARCH64_CC_FLAGS = -DSILICON_PLATFORM=8103
  #*_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES -D HAS_MEMCPY_INTRINSICS


[PcdsFixedAtBuild.common]
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosSystemModel|"MacBook Pro (late 2020)"
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosSystemModelNumber|"MacBook17,1"
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosSystemSku|"MacBook (MacBook17,1)"
  gAppleSiliconPkgTokenSpaceGuid.PcdAppleNumDwc3Controllers|2 # M1 Max case is hardcoded for now.
  gAppleSiliconPkgTokenSpaceGuid.PcdAppleNumDwc3Darts|4 # M1 Max case is hardcoded for now.
  #will be changed later on, default values
  # gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferWidth|1920
  # gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferHeight|1080
  # gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferPixelBpp|30
  
[PcdsDynamicDefault.common]
  # #borrowed from SurfaceDuoPkg
  # gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|1920
  # gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|1080
  # gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|1920
  # gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|1080
  # gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|300
  # gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|50
  # gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|300 
  # gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|50

[Components.common]

MacBookProLate2020Pkg/AcpiTables/DeviceAcpiTables.inf

!include MacBookProTouchBarFamilyPkg/MacBookProTouchBarFamilyPkg.dsc.inc
!include T810XFamilyPkg/T810XFamilyPkg.dsc.inc
!include AppleSiliconPkg/AppleSiliconPkg.dsc.inc
!include AppleSiliconPkg/FrontpageDsc.inc