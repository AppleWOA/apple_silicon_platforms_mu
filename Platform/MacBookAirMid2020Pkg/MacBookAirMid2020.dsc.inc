# M1 DSC include file
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



[BuildOptions.common]
  # use Clang to compile, try to use GCC5 otherwise
  *_CLANG38_AARCH64_CC_FLAGS = -Wno-unused-const-variable -Wno-sometimes-uninitialized -D CLANG
  *_GCC5_AARCH64_CC_FLAGS = -Wno-unused-const-variable -Wno-sometimes-uninitialized -D GCC
  *_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES -D HAS_MEMCPY_INTRINSICS  

[PcdsFeatureFlag.common]

  #MdeModulePkg feature flags
  gEfiMdeModulePkgTokenSpaceGuid.PcdTurnOffUsbLegacySupport|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutGopSupport|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdSupportUpdateCapsuleReset|TRUE

  

  gEmbeddedTokenSpaceGuid.PcdPrePiProduceMemoryTypeInformationHob|TRUE

[PcdsFixedAtBuild.common]
  #ARM architectural fixed build-time settings
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x800000000
  gArmTokenSpaceGuid.PcdArmArchTimerFreqInHz|24000000
  gArmTokenSpaceGuid.PcdCpuVectorBaseAddress|0x8FF000000 #fair warning this location is *totally random*
  gArmTokenSpaceGuid.PcdArmArchTimerSecIntrNum|17 #FIQ timer INTID (needed because Apple timers fire FIQs)
  gArmTokenSpaceGuid.PcdArmArchTimerIntrNum|18 #IRQ timer INTID
#AIC/GIC configuration settings
!if $(AIC_BUILD) == FALSE
  #On all M1 and M2 models, these addresses are completely unmapped and point to nowhere
  #m1n1 will trap accesses to these addresses, which will emulate the vGIC dist/redist
  gArmTokenSpaceGuid.PcdGicDistributorBase|0xf00000000
  gArmTokenSpaceGuid.PcdGicRedistributorsBase|0xf10000000
!else
  #this is MMIO space where the AIC lives
  gAppleSiliconPkgTokenSpaceGuid.PcdAicInterruptControllerBase|0x23b100000

!endif
  #MdeModulePkg build time fixed settings.
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVendor|L"Apple Silicon Windows Project Dev"
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVersionString|L"1.0" 

  #MdePkg fixed build-time settings
  gEfiMdePkgTokenSpaceGuid.PcdMaximumUnicodeStringLength|1000000
  gEfiMdePkgTokenSpaceGuid.PcdMaximumAsciiStringLength|1000000
  gEfiMdePkgTokenSpaceGuid.PcdMaximumLinkedListLength|1000000
  gEfiMdePkgTokenSpaceGuid.PcdDefaultTerminalType|4 #use TTYTERM mode as the default

  #MdeModulePkg ACPI default settings
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemId|"APPLE"
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemTableId|0x494645454c505041
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemRevision|0x00008103
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultCreatorRevision|0x00000001

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
  gEmbeddedTokenSpaceGuid.PcdPrePiCpuIoSize|36
  
  #PCIe settings
  gEfiMdeModulePkgTokenSpaceGuid.PcdSrIovSupport|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdAriSupport|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdMrIovSupport|FALSE

  #OEM Sample FrontPage
  gEfiMdeModulePkgTokenSpaceGuid.PcdBootManagerMenuFile|{ 0x8A, 0x70, 0x42, 0x40, 0x2D, 0x0F, 0x23, 0x48, 0xAC, 0x60, 0x0D, 0x77, 0xB3, 0x11, 0x18, 0x89 }

  #EFI debug settings
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2f
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0xFFFFFFFF

  #variable size
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxVariableSize|0x2000

  gEfiMdePkgTokenSpaceGuid.PcdPlatformBootTimeOut|4

  gEfiMdePkgTokenSpaceGuid.PcdUartDefaultBaudRate|115200

!if $(SECURE_BOOT_ENABLE) == TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxAuthVariableSize|0x2800
  gEfiSecurityPkgTokenSpaceGuid.PcdUserPhysicalPresence|TRUE
!endif

  gM1PkgTokenSpaceGuid.PcdSystemMemoryUefiRegionSize|0x8000000


[PcdsDynamicDefault.common]
  #screen resolution settings
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|2560
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|1600
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|2560
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|1600
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|150
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|150


## Libraries that will be linked into the project
## Most of these are intrinsic to Mu, platform specific things will be noted
## SurfaceDuoPkg/OnePlus6TPkg used as basis for included libraries

[LibraryClasses.common]

  #basic UEFI services
  ArmLib    |ArmPkg/Library/ArmLib/ArmBaseLib.inf
  ArmMmuLib |ArmPkg/Library/ArmMmuLib/ArmMmuBaseLib.inf
  ArmGenericTimerCounterLib  |ArmPkg/Library/ArmGenericTimerPhyCounterLib/ArmGenericTimerPhyCounterLib.inf
  ArmDisassemblerLib|ArmPkg/Library/ArmDisassemblerLib/ArmDisassemblerLib.inf
!if $(AIC_BUILD) == FALSE
  ArmGicLib  |ArmPkg/Drivers/ArmGic/ArmGicLib.inf
!else
  AppleAicLib  |AppleSiliconPkg/Drivers/AppleAicDxe/AppleAicLib.inf
!endif
  ArmHvcLib  |ArmPkg/Library/ArmHvcLib/ArmHvcLib.inf
  ArmSmcLib  |ArmPkg/Library/ArmSmcLibNull/ArmSmcLibNull.inf
  AppleArmGxfHvcLib  |AppleSiliconPkg/Library/AppleArmGxfHvcLib/AppleArmGxfHvcLib.inf
  ArmGicArchLib  |ArmPkg/Library/ArmGicArchLib/ArmGicArchLib.inf
  TimerLib  |ArmPkg/Library/ArmArchTimerLib/ArmArchTimerLib.inf
  BaseLib  |MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib  |MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  BaseSynchronizationLib  |MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  BootLogoLib  |MdeModulePkg/Library/BootLogoLib/BootLogoLib.inf
  BmpSupportLib  |MdeModulePkg/Library/BaseBmpSupportLib/BaseBmpSupportLib.inf
  CapsuleLib  |MdeModulePkg/Library/DxeCapsuleLibNull/DxeCapsuleLibNull.inf
  CacheMaintenanceLib  |ArmPkg/Library/ArmCacheMaintenanceLib/ArmCacheMaintenanceLib.inf
  CompilerIntrinsicsLib  |MdePkg/Library/CompilerIntrinsicsLib/ArmCompilerIntrinsicsLib.inf
  CpuExceptionHandlerLib  |ArmPkg/Library/ArmExceptionLib/ArmExceptionLib.inf
  CpuLib  |MdePkg/Library/BaseCpuLib/BaseCpuLib.inf
  DefaultExceptionHandlerLib|ArmPkg/Library/DefaultExceptionHandlerLib/DefaultExceptionHandlerLib.inf
  DebugAgentLib  |MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf
  DevicePathLib  |MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  DebugLib  |MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
  DebugPrintErrorLevelLib  |MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  DxeServicesLib  |MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  DxeServicesTableLib  |MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
  ExtractGuidedSectionLib  |EmbeddedPkg/Library/PrePiExtractGuidedSectionLib/PrePiExtractGuidedSectionLib.inf
  FileExplorerLib  |MdeModulePkg/Library/FileExplorerLib/FileExplorerLib.inf
  FdtLib  |EmbeddedPkg/Library/FdtLib/FdtLib.inf
  FrameBufferBltLib  |MdeModulePkg/Library/FrameBufferBltLib/FrameBufferBltLib.inf
  HobLib  |MdePkg/Library/DxeHobLib/DxeHobLib.inf
  HiiLib  |MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  IoLib  |MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  LzmaDecompressLib  |MdeModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
  NetLib  |NetworkPkg/Library/DxeNetLib/DxeNetLib.inf
  DpcLib  |NetworkPkg/Library/DxeDpcLib/DxeDpcLib.inf
  UdpIoLib  |NetworkPkg/Library/DxeUdpIoLib/DxeUdpIoLib.inf
  IpIoLib  |NetworkPkg/Library/DxeIpIoLib/DxeIpIoLib.inf
  PcdLib  |MdePkg/Library/DxePcdLib/DxePcdLib.inf
  PeCoffExtraActionLib  |MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  PeCoffLib  |MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  PeCoffGetEntryPointLib  |MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  PerformanceLib  |MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  PrePiLib  |EmbeddedPkg/Library/PrePiLib/PrePiLib.inf
  PrePiHobListPointerLib  |ArmPlatformPkg/Library/PrePiHobListPointerLib/PrePiHobListPointerLib.inf
  PrePiMemoryAllocationLib  |EmbeddedPkg/Library/PrePiMemoryAllocationLib/PrePiMemoryAllocationLib.inf
  PrintLib  |MdePkg/Library/BasePrintLib/BasePrintLib.inf
  RegisterFilterLib|MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf
  ReportStatusCodeLib  |MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  OrderedCollectionLib  |MdePkg/Library/BaseOrderedCollectionRedBlackTreeLib/BaseOrderedCollectionRedBlackTreeLib.inf
  SafeIntLib  |MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf
  SortLib  |MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf
  SynchronizationLib  |MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  UefiLib  |MdePkg/Library/UefiLib/UefiLib.inf
  UefiDecompressLib  |MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf
  UefiBootServicesTableLib  |MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeLib  |MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
  UefiRuntimeServicesTableLib  |MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  UefiUsbLib  |MdePkg/Library/UefiUsbLib/UefiUsbLib.inf
  UefiDriverEntryPoint  |MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiApplicationEntryPoint  |MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiBootManagerLib  |MdeModulePkg/Library/UefiBootManagerLib/UefiBootManagerLib.inf
  UefiHiiServicesLib  |MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  TpmMeasurementLib  |MdeModulePkg/Library/TpmMeasurementLibNull/TpmMeasurementLibNull.inf
  VarCheckLib  |MdeModulePkg/Library/VarCheckLib/VarCheckLib.inf
  VariablePolicyLib  |MdeModulePkg/Library/VariablePolicyLib/VariablePolicyLib.inf
  VariablePolicyHelperLib  |MdeModulePkg/Library/VariablePolicyHelperLib/VariablePolicyHelperLib.inf
  TimeBaseLib  |EmbeddedPkg/Library/TimeBaseLib/TimeBaseLib.inf
  SecureBootVariableLib  |SecurityPkg/Library/SecureBootVariableLib/SecureBootVariableLib.inf
  SecureBootVariableProvisionLib  |SecurityPkg/Library/SecureBootVariableProvisionLib/SecureBootVariableProvisionLib.inf
  RngLib  |MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  IntrinsicLib  |CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
  OpensslLib  |CryptoPkg/Library/OpensslLib/OpensslLibCrypto.inf
  PlatformSecureLib  |SecurityPkg/Library/PlatformSecureLibNull/PlatformSecureLibNull.inf
#services required for UEFI secure boot
!if $(SECURE_BOOT_ENABLE) == TRUE
  AuthVariableLib  |SecurityPkg/Library/AuthVariableLib/AuthVariableLib.inf
!else
  AuthVariableLib|MdeModulePkg/Library/AuthVariableLibNull/AuthVariableLibNull.inf
!endif

  MemoryInitPeiLib|M1Pkg/Library/MemoryInitPeiLib/MemoryInitPeiLib.inf
  PlatformPeiLib|AppleSiliconPkg/Library/PlatformPeiLib/PlatformPeiLib.inf

  BaseBinSecurityLib  |MdePkg/Library/BaseBinSecurityLibNull/BaseBinSecurityLibNull.inf
  DxeMemoryProtectionHobLib  |MdeModulePkg/Library/MemoryProtectionHobLibNull/DxeMemoryProtectionHobLibNull.inf
  PcdDatabaseLoaderLib  |MdeModulePkg/Library/PcdDatabaseLoaderLib/Dxe/PcdDatabaseLoaderLibDxe.inf
  RealTimeClockLib  |EmbeddedPkg/Library/VirtualRealTimeClockLib/VirtualRealTimeClockLib.inf
  ResetUtilityLib  |MdeModulePkg/Library/ResetUtilityLib/ResetUtilityLib.inf
  SecurityLockAuditLib  |MdeModulePkg/Library/SecurityLockAuditLibNull/SecurityLockAuditLibNull.inf
  ResetSystemLib  |AppleSiliconPkg/Library/AppleArmPsciHvcResetLib/AppleArmPsciHvcResetSystem.inf

  SerialPortLib|M1Pkg/Library/AppleUartSerialPortLib/AppleUartSerialPortLib.inf
  CustomizedDisplayLib|MdeModulePkg/Library/CustomizedDisplayLib/CustomizedDisplayLib.inf
  #Platform specific drivers
  HwResetSystemLib|AppleSiliconPkg/Library/AppleArmPsciHvcResetLib/AppleArmPsciHvcResetSystem.inf


  MemoryTypeInfoSecVarCheckLib|MdeModulePkg/Library/MemoryTypeInfoSecVarCheckLib/MemoryTypeInfoSecVarCheckLib.inf
  FltUsedLib|MdePkg/Library/FltUsedLib/FltUsedLib.inf
  MemoryTypeInformationChangeLib|MdeModulePkg/Library/MemoryTypeInformationChangeLibNull/MemoryTypeInformationChangeLibNull.inf
  DtPlatformDtbLoaderLib|EmbeddedPkg/Library/DxeDtPlatformDtbLoaderLibDefault/DxeDtPlatformDtbLoaderLibDefault.inf
  VariableFlashInfoLib|MdeModulePkg/Library/BaseVariableFlashInfoLib/BaseVariableFlashInfoLib.inf
  PlatformPKProtectionLib|SecurityPkg/Library/PlatformPKProtectionLibVarPolicy/PlatformPKProtectionLibVarPolicy.inf

  BaseCryptLib  |CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
  Hash2CryptoLib  |SecurityPkg/Library/BaseHash2CryptoLibNull/BaseHash2CryptoLibNull.inf
  CapsulePersistLib  |MdeModulePkg/Library/CapsulePersistLibNull/CapsulePersistLibNull.inf
  DebugAgentTimerLib  |EmbeddedPkg/Library/DebugAgentTimerLibNull/DebugAgentTimerLibNull.inf

  #DFCI libs
  DfciDeviceIdSupportLib  |DfciPkg/Library/DfciDeviceIdSupportLibNull/DfciDeviceIdSupportLibNull.inf
  DfciGroupLib  |DfciPkg/Library/DfciGroupLibNull/DfciGroups.inf
  DfciRecoveryLib |DfciPkg/Library/DfciRecoveryLib/DfciRecoveryLib.inf
  DfciSettingsLib  |DfciPkg/Library/DfciSettingsLib/DfciSettingsLib.inf
  DfciUiSupportLib  |DfciPkg/Library/DfciUiSupportLibNull/DfciUiSupportLibNull.inf
  DfciV1SupportLib  |DfciPkg/Library/DfciV1SupportLibNull/DfciV1SupportLibNull.inf
  DfciXmlDeviceIdSchemaSupportLib  |DfciPkg/Library/DfciXmlDeviceIdSchemaSupportLib/DfciXmlDeviceIdSchemaSupportLib.inf
  DfciXmlIdentitySchemaSupportLib  |DfciPkg/Library/DfciXmlIdentitySchemaSupportLib/DfciXmlIdentitySchemaSupportLib.inf
  DfciXmlPermissionSchemaSupportLib  |DfciPkg/Library/DfciXmlPermissionSchemaSupportLib/DfciXmlPermissionSchemaSupportLib.inf
  DfciXmlSettingSchemaSupportLib  |DfciPkg/Library/DfciXmlSettingSchemaSupportLib/DfciXmlSettingSchemaSupportLib.inf
  DfciSettingChangedNotificationLib  |DfciPkg/Library/DfciSettingChangedNotificationLib/DfciSettingChangedNotificationLibNull.inf


  FileHandleLib  |MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
  FmpAuthenticationLib  |SecurityPkg/Library/FmpAuthenticationLibPkcs7/FmpAuthenticationLibPkcs7.inf
  HttpLib  |NetworkPkg/Library/DxeHttpLib/DxeHttpLib.inf
  JsonLiteParserLib  |MsCorePkg/Library/JsonLiteParser/JsonLiteParser.inf
  MsPlatformDevicesLib  |M1Pkg/Library/MsPlatformDevicesLib/MsPlatformDevicesLib.inf
  MsSecureBootModeSettingLib  |OemPkg/Library/MsSecureBootModeSettingLib/MsSecureBootModeSettingLib.inf
  MuTelemetryHelperLib  |MsWheaPkg/Library/MuTelemetryHelperLib/MuTelemetryHelperLib.inf
  NonDiscoverableDeviceRegistrationLib  |MdeModulePkg/Library/NonDiscoverableDeviceRegistrationLib/NonDiscoverableDeviceRegistrationLib.inf
  OemHookStatusCodeLib  |MdeModulePkg/Library/OemHookStatusCodeLibNull/OemHookStatusCodeLibNull.inf
  PasswordPolicyLib  |OemPkg/Library/PasswordPolicyLib/PasswordPolicyLib.inf
  PlatformBootManagerLib  |MsCorePkg/Library/PlatformBootManagerLib/PlatformBootManagerLib.inf

!if $(SECURE_BOOT_ENABLE) == TRUE
  SecureBootKeyStoreLib  |M1Pkg/Library/SecureBootKeyStoreLib/SecureBootKeyStoreLib.inf
!else
  SecureBootKeyStoreLib  |MsCorePkg/Library/SecureBootKeyStoreLibNull/SecureBootKeyStoreLibNull.inf
!endif

  PlatformThemeLib  |MsGraphicsPkg/Library/SamplePlatformThemeLib/PlatformThemeLib.inf
  ReportStatusCodeLib  |MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  ShellLib  |ShellPkg/Library/UefiShellLib/UefiShellLib.inf
  UefiDevicePathLib  |MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  UefiScsiLib  |MdePkg/Library/UefiScsiLib/UefiScsiLib.inf
  ZeroTouchSettingsLib  |ZeroTouchPkg/Library/ZeroTouchSettings/ZeroTouchSettings.inf
  MuSecureBootKeySelectorLib  |MsCorePkg/Library/MuSecureBootKeySelectorLib/MuSecureBootKeySelectorLib.inf
  UpdateFacsHardwareSignatureLib  |PcBdsPkg/Library/UpdateFacsHardwareSignatureLib/UpdateFacsHardwareSignatureLib.inf


[LibraryClasses.common.SEC]

  HobLib  |EmbeddedPkg/Library/PrePiHobLib/PrePiHobLib.inf
  MemoryAllocationLib  |EmbeddedPkg/Library/PrePiMemoryAllocationLib/PrePiMemoryAllocationLib.inf
  PrePiMemoryAllocationLib  |EmbeddedPkg/Library/PrePiMemoryAllocationLib/PrePiMemoryAllocationLib.inf
  PrePiHobListPointerLib  |ArmPlatformPkg/Library/PrePiHobListPointerLib/PrePiHobListPointerLib.inf
  PcdLib  |MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  BaseMemoryLib  |MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf

[LibraryClasses.common.DXE_CORE]
  HobLib  |MdePkg/Library/DxeCoreHobLib/DxeCoreHobLib.inf
  MemoryAllocationLib  |MdeModulePkg/Library/DxeCoreMemoryAllocationLib/DxeCoreMemoryAllocationLib.inf
  DxeCoreEntryPoint  |MdePkg/Library/DxeCoreEntryPoint/DxeCoreEntryPoint.inf
  ReportStatusCodeLib  |MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  ExtractGuidedSectionLib  |MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  UefiDecompressLib  |MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf
  PerformanceLib  |MdeModulePkg/Library/DxeCorePerformanceLib/DxeCorePerformanceLib.inf
  DxeServicesLib  |MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  MsUiThemeLib                  |MsGraphicsPkg/Library/MsUiThemeLib/Dxe/MsUiThemeLib.inf
  MsPlatformEarlyGraphicsLib    |MsGraphicsPkg/Library/MsEarlyGraphicsLibNull/Dxe/MsEarlyGraphicsLibNull.inf
  PciSegmentLib|MdePkg/Library/BasePciSegmentLibPci/BasePciSegmentLibPci.inf
  PciLib|MdePkg/Library/BasePciLibPciExpress/BasePciLibPciExpress.inf
  PciExpressLib|MdePkg/Library/BasePciExpressLib/BasePciExpressLib.inf

[LibraryClasses.common.DXE_DRIVER]
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  SecurityManagementLib|MdeModulePkg/Library/DxeSecurityManagementLib/DxeSecurityManagementLib.inf
  PerformanceLib|MdeModulePkg/Library/DxePerformanceLib/DxePerformanceLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  PciSegmentLib|MdePkg/Library/BasePciSegmentLibPci/BasePciSegmentLibPci.inf
  PciLib|MdePkg/Library/BasePciLibPciExpress/BasePciLibPciExpress.inf
  PciExpressLib|MdePkg/Library/BasePciExpressLib/BasePciExpressLib.inf

!if $(SECURE_BOOT_ENABLE) == TRUE
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
!endif
  MsUiThemeLib                  |MsGraphicsPkg/Library/MsUiThemeLib/Dxe/MsUiThemeLib.inf
  MsPlatformEarlyGraphicsLib    |MsGraphicsPkg/Library/MsEarlyGraphicsLibNull/Dxe/MsEarlyGraphicsLibNull.inf

[LibraryClasses.common.UEFI_APPLICATION]
  UefiDecompressLib  |MdePkg/Library/BaseUefiDecompressLib/BaseUefiTianoCustomDecompressLib.inf
  PerformanceLib  |MdeModulePkg/Library/DxePerformanceLib/DxePerformanceLib.inf
  MemoryAllocationLib  |MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  HiiLib  |MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  ShellLib  |ShellPkg/Library/UefiShellLib/UefiShellLib.inf
  FileHandleLib  |MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
  ReportStatusCodeLib  |MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  ExtractGuidedSectionLib  |MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  MsUiThemeLib                  |MsGraphicsPkg/Library/MsUiThemeLib/Dxe/MsUiThemeLib.inf
  MsPlatformEarlyGraphicsLib    |MsGraphicsPkg/Library/MsEarlyGraphicsLibNull/Dxe/MsEarlyGraphicsLibNull.inf
  PciSegmentLib|MdePkg/Library/BasePciSegmentLibPci/BasePciSegmentLibPci.inf
  PciLib|MdePkg/Library/BasePciLibPciExpress/BasePciLibPciExpress.inf
  PciExpressLib|MdePkg/Library/BasePciExpressLib/BasePciExpressLib.inf

!if $(SECURE_BOOT_ENABLE) == TRUE
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
!endif

[LibraryClasses.common.UEFI_DRIVER]
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  UefiDecompressLib|MdePkg/Library/BaseUefiDecompressLib/BaseUefiTianoCustomDecompressLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  PerformanceLib|MdeModulePkg/Library/DxePerformanceLib/DxePerformanceLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  UefiScsiLib|MdePkg/Library/UefiScsiLib/UefiScsiLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  MsUiThemeLib  |MsGraphicsPkg/Library/MsUiThemeLib/Dxe/MsUiThemeLib.inf
  MsPlatformEarlyGraphicsLib  |MsGraphicsPkg/Library/MsEarlyGraphicsLibNull/Dxe/MsEarlyGraphicsLibNull.inf
  PciSegmentLib|MdePkg/Library/BasePciSegmentLibPci/BasePciSegmentLibPci.inf
  PciLib|MdePkg/Library/BasePciLibPciExpress/BasePciLibPciExpress.inf
  PciExpressLib|MdePkg/Library/BasePciExpressLib/BasePciExpressLib.inf

!if $(SECURE_BOOT_ENABLE) == TRUE
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
!endif

[LibraryClasses.common.DXE_RUNTIME_DRIVER]
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  CapsuleLib|MdeModulePkg/Library/DxeCapsuleLibNull/DxeCapsuleLibNull.inf
  ArmHvcLib|ArmPkg/Library/ArmHvcLib/ArmHvcLib.inf
  AppleArmGxfHvcLib|AppleSiliconPkg/Library/AppleArmGxfHvcLib/AppleArmGxfHvcLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf

  # System Reset
  HwResetSystemLib|AppleSiliconPkg/Library/AppleArmPsciHvcResetLib/AppleArmPsciHvcResetSystem.inf

  VariablePolicyLib|MdeModulePkg/Library/VariablePolicyLib/VariablePolicyLibRuntimeDxe.inf

!if $(SECURE_BOOT_ENABLE) == TRUE
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/RuntimeCryptLib.inf
!endif
  MsUiThemeLib                  |MsGraphicsPkg/Library/MsUiThemeLib/Dxe/MsUiThemeLib.inf
  MsPlatformEarlyGraphicsLib    |MsGraphicsPkg/Library/MsEarlyGraphicsLibNull/Dxe/MsEarlyGraphicsLibNull.inf



#Most parts borrowed or used from other platforms
[Components.common]

  #SEC phase
  M1Pkg/PrePi/PrePi.inf {
    <LibraryClasses>
    SerialPortLib|AppleSiliconPkg/Library/AppleUartSerialPortLib/AppleUartSerialPortLib.inf
  }



  #DXE phase
  MdeModulePkg/Core/Dxe/DxeMain.inf {
    <LibraryClasses>
    NULL|MdeModulePkg/Library/DxeCrc32GuidedSectionExtractLib/DxeCrc32GuidedSectionExtractLib.inf
    NULL|MdeModulePkg/Library/DxeCrc32GuidedSectionExtractLib/DxeCrc32GuidedSectionExtractLib.inf
    NULL|MdeModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
  }


  #PCD database
  MdeModulePkg/Universal/PCD/Dxe/Pcd.inf {
    <LibraryClasses>
    PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }

  #ARM architectural services

  ArmPkg/Drivers/CpuDxe/CpuDxe.inf
  ArmPkg/Drivers/TimerDxe/TimerDxe.inf
!if $(AIC_BUILD) == FALSE
  ArmPkg/Drivers/ArmGic/ArmGicDxe.inf
!else
  AppleSiliconPkg/Drivers/AppleAicDxe/AppleAicDxe.inf
!endif
  MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf
  EmbeddedPkg/MetronomeDxe/MetronomeDxe.inf
  EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf

  # EFI Runtime Services (we can put PSCI here too if need be)
  MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
  MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  EmbeddedPkg/EmbeddedMonotonicCounter/EmbeddedMonotonicCounter.inf

  # Fake Variable Services
  MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf

  MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
  MdeModulePkg/Universal/Console/TerminalDxe/TerminalDxe.inf
  MdeModulePkg/Universal/SerialDxe/SerialDxe.inf

  # Security Stub
  MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf {
    <LibraryClasses>
!if $(SECURE_BOOT_ENABLE) == TRUE
      NULL|SecurityPkg/Library/DxeImageVerificationLib/DxeImageVerificationLib.inf
!endif
  }

!if $(SECURE_BOOT_ENABLE) == TRUE
  SecurityPkg/VariableAuthenticated/SecureBootConfigDxe/SecureBootConfigDxe.inf
  #M1Pkg/Driver/SecureBootProvisioningDxe/SecureBootProvisioningDxe.inf
!endif

  MsCorePkg/MuVarPolicyFoundationDxe/MuVarPolicyFoundationDxe.inf

  # PCIe
  #MdeModulePkg/Bus/Pci/PciBusDxe/PciBusDxe.inf
  #MdeModulePkg/Bus/Pci/PciHostBridgeDxe/PciHostBridgeDxe.inf


  # Disk IO
  #M1Pkg TODO: ANS2/SART drivers 
  MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
  MdeModulePkg/Universal/FvSimpleFileSystemDxe/FvSimpleFileSystemDxe.inf
  FatPkg/EnhancedFatDxe/Fat.inf
  MdeModulePkg/Bus/Scsi/ScsiBusDxe/ScsiBusDxe.inf
  MdeModulePkg/Bus/Scsi/ScsiDiskDxe/ScsiDiskDxe.inf
  #MdeModulePkg/Bus/Pci/NvmExpressDxe/NvmExpressDxe.inf

  # USB 2.0/3.0 support (this should be OOB working since Apple uses a Synopsys controller)
  # required for booting off USB mass storage devices
  MdeModulePkg/Bus/Usb/UsbMouseAbsolutePointerDxe/UsbMouseAbsolutePointerDxe.inf
  MdeModulePkg/Bus/Usb/UsbMassStorageDxe/UsbMassStorageDxe.inf
  MdeModulePkg/Bus/Usb/UsbBusDxe/UsbBusDxe.inf
  MdeModulePkg/Bus/Usb/UsbKbDxe/UsbKbDxe.inf
  #MdeModulePkg/Bus/Pci/UhciDxe/UhciDxe.inf
  #MdeModulePkg/Bus/Pci/EhciDxe/EhciDxe.inf
  #MdeModulePkg/Bus/Pci/XhciDxe/XhciDxe.inf

  # DT
  EmbeddedPkg/Drivers/DtPlatformDxe/DtPlatformDxe.inf

  EmbeddedPkg/Drivers/VirtualKeyboardDxe/VirtualKeyboardDxe.inf

  # BDS
  MdeModulePkg/Universal/DisplayEngineDxe/DisplayEngineDxe.inf
  MdeModulePkg/Universal/PrintDxe/PrintDxe.inf
  MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf
  MdeModulePkg/Universal/SerialDxe/SerialDxe.inf
  MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
  MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf
  MdeModulePkg/Universal/SmbiosDxe/SmbiosDxe.inf
  MdeModulePkg/Universal/SetupBrowserDxe/SetupBrowserDxe.inf
  MdeModulePkg/Universal/DriverHealthManagerDxe/DriverHealthManagerDxe.inf
  MdeModulePkg/Universal/BdsDxe/BdsDxe.inf


  #ACPI (note: only the essentials for now, ASL tables in progress)
  MdeModulePkg/Universal/Acpi/AcpiTableDxe/AcpiTableDxe.inf
  MdeModulePkg/Universal/Acpi/AcpiPlatformDxe/AcpiPlatformDxe.inf
  MdeModulePkg/Universal/Acpi/BootGraphicsResourceTableDxe/BootGraphicsResourceTableDxe.inf

  #SMBIOS information
  M1Pkg/Library/SmbiosInfoDxe/SmbiosInfoDxe.inf

  #Miscellaneous
  MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/ReportStatusCodeRouterRuntimeDxe.inf
  MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/StatusCodeHandlerRuntimeDxe.inf
  MsCorePkg/Universal/StatusCodeHandler/Serial/Dxe/SerialStatusCodeHandlerDxe.inf
  EmbeddedPkg/SimpleTextInOutSerial/SimpleTextInOutSerial.inf
  MdeModulePkg/Universal/ResetSystemRuntimeDxe/ResetSystemRuntimeDxe.inf
!if $(SECURE_BOOT_ENABLE) == TRUE
  MsCorePkg/MuCryptoDxe/MuCryptoDxe.inf
  CryptoPkg/Driver/CryptoDxe.inf
!endif
  MsCorePkg/MuVarPolicyFoundationDxe/MuVarPolicyFoundationDxe.inf

