
/**
 * @file AppleAicDxe.h
 * @author amarioguy (Arminder Singh)
 * @brief AIC DXE Driver Header File.
 * 
 * @version 1.0
 * @date 2022-09-24
 * 
 * @copyright Copyright (c) amarioguy (Arminder Singh) 2022.
 * 
 */

#ifndef APPLE_AIC_DXE_H
#define APPLE_AIC_DXE_H

#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/ArmLib.h>
#include <Library/AppleAicLib.h>

#include <Protocol/Cpu.h>
#include <Protocol/HardwareInterrupt.h>
#include <Protocol/HardwareInterrupt2.h>


extern HARDWARE_INTERRUPT_HANDLER  *AicRegisteredInterruptHandlers;

// Common API
EFI_STATUS
InstallAndRegisterInterruptService (
  IN EFI_HARDWARE_INTERRUPT_PROTOCOL   *InterruptProtocol,
  IN EFI_HARDWARE_INTERRUPT2_PROTOCOL  *Interrupt2Protocol,
  IN EFI_CPU_INTERRUPT_HANDLER         InterruptHandler,
  IN EFI_EVENT_NOTIFY                  ExitBootServicesEvent
  );

EFI_STATUS
EFIAPI
RegisterInterruptSource (
  IN EFI_HARDWARE_INTERRUPT_PROTOCOL  *This,
  IN HARDWARE_INTERRUPT_SOURCE        Source,
  IN HARDWARE_INTERRUPT_HANDLER       Handler
  );


//TODO: AICv1 API

EFI_STATUS AppleAicV1DxeInit(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable);

//AICv2 API

EFI_STATUS AppleAicV2DxeInit(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable, APPLE_AIC_VERSION aicVersion);


#endif //APPLE_AIC_DXE_H