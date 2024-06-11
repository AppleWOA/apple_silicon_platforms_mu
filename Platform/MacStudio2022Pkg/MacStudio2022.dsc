#Mac Studio 2022 UEFI DSC file
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
  PLATFORM_NAME                  = MacStudio2022
  PLATFORM_GUID                  = d7d529ec-4827-494a-85e7-5900d9841a9e
  PLATFORM_VERSION               = 1.0
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/MacStudio2022-$(ARCH)
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = MacStudio2022Pkg/MacStudio2022.fdf
  SECURE_BOOT_ENABLE             = FALSE #disable secure boot for now
  AIC_BUILD                      = TRUE #AIC build enabled by default, change to false if you want to use a vGIC
  NETWORK_TLS_ENABLE             = TRUE


[BuildOptions.common]
  GCC:*_*_AARCH64_CC_FLAGS = -DSILICON_PLATFORM=6000
  *_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES -D HAS_MEMCPY_INTRINSICS


[PcdsFixedAtBuild.common]
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosSystemModel|"Mac Studio (2022)"
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosSystemModelNumber|"Mac13,1/Mac13,2"
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosSystemSku|"Mac Studio (Mac13,1/Mac13,2)"
  gAppleSiliconPkgTokenSpaceGuid.PcdSystemMemoryUefiRegionSize|0x8000000
  #will be changed later on, default values
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferWidth|1920
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferHeight|1080
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferPixelBpp|30
  
[PcdsDynamicDefault.common]
  #borrowed from SurfaceDuoPkg
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|1920
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|1080
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|1920
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|1080
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|300
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|50
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|300 
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|50

[Components.common]

  MacStudio2022Pkg/AcpiTables/DeviceAcpiTables.inf
  AppleSiliconPkg/Drivers/AppleBootTimeEmbeddedFirmwareHelperDxe/AppleBootTimeEmbeddedFirmwareHelperDxe.inf


!include T600XFamilyPkg/T600XFamilyPkg.dsc.inc
!include AppleSiliconPkg/AppleSiliconPkg.dsc.inc
!include AppleSiliconPkg/FrontpageDsc.inc