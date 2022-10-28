/**
 * @file AppleAicCommonDxe.c
 * @author amarioguy (Arminder Singh)
 * 
 * AIC DXE Driver code common to both AICv1 and AICv2
 * @version 1.0
 * @date 2022-10-06
 * 
 * @copyright Copyright (c) amarioguy (Arminder Singh), 2022.
 * 
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 * 
 */

#include "AppleAicDxe.h"

EFI_HANDLE  gHardwareInterruptHandle = NULL;

EFI_EVENT  EfiExitBootServicesEvent = (EFI_EVENT)NULL;

HARDWARE_INTERRUPT_HANDLER  *AicRegisteredInterruptHandlers = NULL;

STATIC VOID  *mCpuArchProtocolNotifyEventRegistration;

VOID
EFIAPI
ExitBootServicesEvent (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  );

//borrowed from ArmGicDxe as it does the job well enough
/**
  Register Handler for the specified interrupt source.

  @param This     Instance pointer for this protocol
  @param Source   Hardware source of the interrupt
  @param Handler  Callback for interrupt. NULL to unregister

  @retval EFI_SUCCESS Source was updated to support Handler.
  @retval EFI_DEVICE_ERROR  Hardware could not be programmed.

**/
EFI_STATUS
EFIAPI
RegisterInterruptSource (
  IN EFI_HARDWARE_INTERRUPT_PROTOCOL  *This,
  IN HARDWARE_INTERRUPT_SOURCE        Source,
  IN HARDWARE_INTERRUPT_HANDLER       Handler
  )
{
  if (Source >= AicInfoStruct->MaxIrqs) {
    ASSERT (FALSE);
    return EFI_UNSUPPORTED;
  }

  if ((Handler == NULL) && (AicRegisteredInterruptHandlers[Source] == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((Handler != NULL) && (AicRegisteredInterruptHandlers[Source] != NULL)) {
    return EFI_ALREADY_STARTED;
  }

  AicRegisteredInterruptHandlers[Source] = Handler;

  // If the interrupt handler is unregistered then disable the interrupt
  if (NULL == Handler) {
    return This->DisableInterruptSource (This, Source);
  } else {
    return This->EnableInterruptSource (This, Source);
  }
}


//inspired/borrowed from ArmGicCommonDxe
/**
 * Notifies the Interrupt Service registration task when the CPU protocol becomes available.
 * 
 * @param Event 
 * @param Context 
 */
STATIC VOID EFIAPI CpuArchProtocolNotify(IN EFI_EVENT Event, IN VOID *Context)
{
  EFI_CPU_ARCH_PROTOCOL  *CpuProtocol;
  EFI_STATUS             Status;

  // Get the required CPU protocol
  Status = gBS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, (VOID **)&CpuProtocol);
  if (EFI_ERROR (Status)) {
    return;
  }

  //unregister the default FIQ and IRQ handler for the CPU protocol
  Status = CpuProtocol->RegisterInterruptHandler (CpuProtocol, ARM_ARCH_EXCEPTION_IRQ, NULL);
  Status = CpuProtocol->RegisterInterruptHandler (CpuProtocol, EXCEPT_AARCH64_FIQ, NULL);
  if (EFI_ERROR (Status)) {
    DEBUG((DEBUG_ERROR, "%a: Unregistering default exception handler failed!! Status: 0x%llx\n", __FUNCTION__, Status));
    return;
  }

  //now register the new handlers
  Status = CpuProtocol->RegisterInterruptHandler (CpuProtocol, ARM_ARCH_EXCEPTION_IRQ, (EFI_CPU_INTERRUPT_HANDLER)Context);
  Status = CpuProtocol->RegisterInterruptHandler (CpuProtocol, EXCEPT_AARCH64_FIQ, (EFI_CPU_INTERRUPT_HANDLER)Context);
  if (EFI_ERROR (Status)) {
    DEBUG((DEBUG_ERROR, "%a: Registering new exception handler failed!! Status: 0x%llx\n", __FUNCTION__, Status));
    return;
  }
  gBS->CloseEvent (Event);

}


EFI_STATUS
InstallAndRegisterInterruptService (
  IN EFI_HARDWARE_INTERRUPT_PROTOCOL   *InterruptProtocol,
  IN EFI_HARDWARE_INTERRUPT2_PROTOCOL  *Interrupt2Protocol,
  IN EFI_CPU_INTERRUPT_HANDLER         InterruptHandler,
  IN EFI_EVENT_NOTIFY                  ExitBootServicesEvent
  )
{
    EFI_STATUS Status;
    CONST UINTN InterruptHandlersSize = (sizeof(HARDWARE_INTERRUPT_HANDLER) * AicInfoStruct->NumIrqs);

    //set up RAM for IRQ handlers
    AicRegisteredInterruptHandlers = AllocateZeroPool(InterruptHandlersSize);
    if(AicRegisteredInterruptHandlers == NULL)
    {
        return EFI_OUT_OF_RESOURCES;
    }

    Status = gBS->InstallMultipleProtocolInterfaces (
                  &gHardwareInterruptHandle,
                  &gHardwareInterruptProtocolGuid,
                  InterruptProtocol,
                  &gHardwareInterrupt2ProtocolGuid,
                  Interrupt2Protocol,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //Install the IRQ/FIQ handlers when the CPU architecture protocol is ready.
  EfiCreateProtocolNotifyEvent(&gEfiCpuArchProtocolGuid, TPL_CALLBACK, CpuArchProtocolNotify, (VOID *)InterruptHandler, &mCpuArchProtocolNotifyEventRegistration);
  
  //set up the ExitBootServices event
  Status = gBS->CreateEvent (
                  EVT_SIGNAL_EXIT_BOOT_SERVICES,
                  TPL_NOTIFY,
                  ExitBootServicesEvent,
                  NULL,
                  &EfiExitBootServicesEvent
                  );
  
  return Status;
    
} 