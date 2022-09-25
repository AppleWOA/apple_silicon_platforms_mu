/**
 * @file AppleAicDxe.c
 * @author amarioguy (arminders208@outlook.com)
 * 
 * This file implements the selector logic for the AIC DXE Driver.
 * 
 * @version 1.0
 * @date 2022-08-16
 * 
 * @copyright Copyright (c) amarioguy (Arminder Singh) 2022.
 * 
 * Credits to the Asahi Linux contributors for the initial AIC implementations in m1n1 and linux.
 * 
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 * 
 */

#include <PiDxe.h>

#include "AppleAicDxe.h"

/**
 * Initialize the AIC protocol.
 * 
 * @param ImageHandle 
 * @param SystemTable 
 * @return EFI_STATUS 
 */

EFI_STATUS
InterruptDxeInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;
  APPLE_AIC_VERSION Version;

  Version = AppleArmGetAicVersion();
  return EFI_SUCCESS;
}