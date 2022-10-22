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
    
} 