APRIORI DXE {
  INF MdeModulePkg/Core/Dxe/DxeMain.inf
  INF MdeModulePkg/Universal/PCD/Dxe/Pcd.inf
  INF ArmPkg/Drivers/CpuDxe/CpuDxe.inf
!if $(AIC_BUILD) == FALSE
  INF ArmPkg/Drivers/ArmGic/ArmGicDxe.inf
!else 
  INF AppleSiliconPkg/Drivers/AppleAicDxe/AppleAicDxe.inf
!endif
  INF EmbeddedPkg/MetronomeDxe/MetronomeDxe.inf
  INF ArmPkg/Drivers/TimerDxe/TimerDxe.inf
  INF MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/ReportStatusCodeRouterRuntimeDxe.inf
  INF MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/StatusCodeHandlerRuntimeDxe.inf
  INF MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf
  INF AppleSiliconPkg/Drivers/SimpleFbDxe/SimpleFbDxe.inf
  INF MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  INF MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf
  INF MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
  #INF AppleSiliconPkg/Drivers/AppleDartIoMmuDxe/AppleDartIoMmuDxe.inf
  INF AppleSiliconPkg/Drivers/AppleEmbeddedGpioControllerDxe/AppleEmbeddedGpioControllerDxe.inf
  INF AppleSiliconPkg/Drivers/AppleSiliconPciPlatformDxe/AppleSiliconPciPlatformDxe.inf
}