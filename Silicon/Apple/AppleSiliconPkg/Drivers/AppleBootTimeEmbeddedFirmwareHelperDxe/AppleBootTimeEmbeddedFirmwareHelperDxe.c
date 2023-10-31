/**
 * Copyright (c) 2023, amarioguy (AppleWOA authors).
 * 
 * Module Name:
 *     AppleBootTimeEmbeddedFirmwareHelperDxe.c
 * 
 * Abstract:
 *     Helper driver for Apple Silicon platforms to load firmware blobs required for onboard PCIe or
 *     platform devices.
 *     Parts of the driver borrowed from Linaro/OpenPlatformPkg, RenesasFirmwarePD720202.c
 *     XHCI driver load based on code from Asahi Linux kernel and U-Boot trees
 * 
 * Environment:
 *     UEFI DXE (Driver Execution Environment).
 * 
 * License:
 *     SPDX-License-Identifier: BSD-2-Clause-Patent OR MIT
 * 
*/

#include <PiDxe.h>
#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/ArmLib.h>
#include <Library/PrintLib.h>
#include <Library/PcdLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/PciLib.h>
#include <Library/PciSegmentLib.h>
#include <Library/PciExpressLib.h>
#include <IndustryStandard/Pci.h>
#include <Library/TimerLib.h>

#include <Protocol/PciIo.h>
#include <Drivers/AppleBootTimeEmbeddedFirmwareHelperXhciDxe.h>

//
// This driver's main purpose is to load firmware blobs embedded in the FV
// for testing purposes during Apple platform bringup, since Apple relies
// on the AP uploading firmware blobs to the IOPs/coprocessors, and NAND storage isn't guaranteed
// to be available. (will probably also be useful for the remote boot case on embedded, where an
// Asahi Linux EFI system partition is not guaranteed to be available.)
//
// Right now, it only handles the USB firmware, but can be extended to other firmware blobs
// if it is deemed necessary.
//


//
// Global variables.
// Mainly for recording if a firmware blob is found and should be loaded.
//
BOOLEAN gUsbFirmwareFound = FALSE;
VOID *UsbFirmwarePointer;
VOID *NewUsbFirmwareBlobPointer;
UINTN UsbFirmwareSize;

//
// *********************************************************
// ASMedia XHCI controller specific firmware upload section.
// *********************************************************
//

STATIC UINT8 AppleAsmediaReadRegister(IN EFI_PCI_IO_PROTOCOL *PciIoProtocolInstance,
  IN UINT16 Address)
{
  EFI_STATUS Status;
  UINT8 RegisterStatus;
  UINT8 RegisterValue;

  Status = PciIoProtocolInstance->PollMem(PciIoProtocolInstance, EfiPciIoWidthUint8, PCI_BAR_IDX0, ASMEDIA_REGISTER_STATUS, ASMEDIA_REGISTER_STATUS_BUSY, 0, 100000, ((UINT64 *)&RegisterStatus));
  
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_INFO, "%a - Timed out when polling ASMEDIA_REGISTER_STATUS PCI register for read register wait op! Status - %r\n", __FUNCTION__, Status));
    ASSERT_EFI_ERROR (Status);
  }

  Status = PciIoProtocolInstance->Mem.Write(PciIoProtocolInstance, EfiPciIoWidthUint16, PCI_BAR_IDX0, ASMEDIA_REGISTER_ADDR, 1, ((VOID *)&Address));
  
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_INFO, "%a - Failed to write address to ASMEDIA_REG_ADDR PCI register! Status - %r\n", __FUNCTION__, Status));
    ASSERT_EFI_ERROR (Status);
  }

  Status = PciIoProtocolInstance->PollMem(PciIoProtocolInstance, EfiPciIoWidthUint8, PCI_BAR_IDX0, ASMEDIA_REGISTER_STATUS, ASMEDIA_REGISTER_STATUS_BUSY, 0, 100000, ((UINT64 *)&RegisterStatus));
  
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_INFO, "%a - Timed out when polling ASMEDIA_REGISTER_STATUS PCI register for read register address op! Status - %r\n", __FUNCTION__, Status));
    ASSERT_EFI_ERROR (Status);
  }

  Status = PciIoProtocolInstance->Mem.Read(PciIoProtocolInstance, EfiPciIoWidthUint8, PCI_BAR_IDX0, ASMEDIA_REGISTER_READ_DATA, 1, ((VOID *)&RegisterValue));
  
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_INFO, "%a - Failed to reaed address from ASMEDIA_REG_ADDR PCI register! Status - %r\n", __FUNCTION__, Status));
    ASSERT_EFI_ERROR (Status);
  }
  return RegisterValue;

}

STATIC VOID AppleAsmediaWriteRegister(IN EFI_PCI_IO_PROTOCOL *PciIoProtocolInstance,
 IN UINT16 Address, 
 IN UINT8 Data, 
 IN BOOLEAN Wait) 
 {
  EFI_STATUS Status;
  UINT8 RegisterStatus;
  UINT32 i = 0;
  Status = PciIoProtocolInstance->Mem.Write(PciIoProtocolInstance, EfiPciIoWidthUint16, PCI_BAR_IDX0, ASMEDIA_REGISTER_ADDR, 1, &Address);
  
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_INFO, "%a - Failed to write address to ASMEDIA_REG_ADDR PCI register! Status - %r\n", __FUNCTION__, Status));
    ASSERT_EFI_ERROR (Status);
  }

  Status = PciIoProtocolInstance->PollMem(PciIoProtocolInstance, EfiPciIoWidthUint8, PCI_BAR_IDX0, ASMEDIA_REGISTER_STATUS, ASMEDIA_REGISTER_STATUS_BUSY, 0, 100000, ((UINT64 *)&RegisterStatus));
  
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_INFO, "%a - Timed out when polling ASMEDIA_REGISTER_STATUS PCI register for write register address op! Status - %r\n", __FUNCTION__, Status));
    ASSERT_EFI_ERROR (Status);
  }
  
  Status = PciIoProtocolInstance->Mem.Write(PciIoProtocolInstance, EfiPciIoWidthUint8, PCI_BAR_IDX0, ASMEDIA_REGISTER_WRITE_DATA, 1, &Data);
  
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_INFO, "%a - Failed to write data to ASMEDIA_REG_WRITE_DATA PCI register! Status - %r\n", __FUNCTION__, Status));
    ASSERT_EFI_ERROR (Status);
  }
  
  Status = PciIoProtocolInstance->PollMem(PciIoProtocolInstance, EfiPciIoWidthUint8, PCI_BAR_IDX0, ASMEDIA_REGISTER_STATUS, ASMEDIA_REGISTER_STATUS_BUSY, 0, 100000, ((UINT64 *)&RegisterStatus));
  
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_INFO, "%a - Timed out when polling ASMEDIA_REGISTER_STATUS PCI register for write register data op! Status - %r\n", __FUNCTION__, Status));
    ASSERT_EFI_ERROR (Status);
  }

  if(Wait == FALSE) {
    DEBUG((DEBUG_INFO, "%a - Register write successful\n"));
    return;
  }
  
  for(i = 0; i < TIMEOUT_USEC; i++) {
    if (AppleAsmediaReadRegister(PciIoProtocolInstance, Address) == Data) {
      break;
    }
  }
  
  if (i >= TIMEOUT_USEC) {
    DEBUG((DEBUG_INFO, "%a - Register verify timed out!\n", __FUNCTION__));
    ASSERT(FALSE);
  }
 }


STATIC EFI_STATUS AppleAsmediaSendMessage(IN EFI_PCI_IO_PROTOCOL *PciIoProtocolInstance, UINT64 MessageToSend) {
    UINT8 Operation;
    UINT32 MessageLow = (UINT32)MessageToSend;
    UINT32 MessageHigh = (MessageToSend >> 32);
    CONST UINT8 WriteOperation = ASMEDIA_CONFIGURATION_CONTROL_WRITE_BIT;
    EFI_STATUS Status;

    for(INT32 i = 0; i < TIMEOUT_USEC; i++) {
        Status = PciIoProtocolInstance->Pci.Read(PciIoProtocolInstance, EfiPciIoWidthUint8, ASMEDIA_CONFIGURATION_CONTROL_REG, 1, &Operation);
        if(!(Operation & ASMEDIA_CONFIGURATION_CONTROL_WRITE_BIT)) {
            break;
        }
        MicroSecondDelay(1);
    }
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_INFO, "%a - Failed to read operation message from mailbox. Status - %r\n", __FUNCTION__, Status));
      ASSERT_EFI_ERROR(Status);
    }
    if(Operation & ASMEDIA_CONFIGURATION_CONTROL_WRITE_BIT) {
        DEBUG((DEBUG_INFO, "%a: Mailbox write timed out, data to write: 0x%llx\n", __FUNCTION__, MessageToSend));
        return EFI_TIMEOUT;
    }
    Status = PciIoProtocolInstance->Pci.Write(PciIoProtocolInstance, EfiPciIoWidthUint32, ASMEDIA_CONFIGURATION_DATA_WRITE_REG_0, 1, ((VOID *)&MessageLow));
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_INFO, "%a - Failed to write write data message to mailbox. Status - %r\n", __FUNCTION__, Status));
      ASSERT_EFI_ERROR(Status);
    }
    Status = PciIoProtocolInstance->Pci.Write(PciIoProtocolInstance, EfiPciIoWidthUint32, ASMEDIA_CONFIGURATION_DATA_WRITE_REG_1, 1, ((VOID *)&MessageHigh));
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_INFO, "%a - Failed to write write data message to mailbox. Status - %r\n", __FUNCTION__, Status));
      ASSERT_EFI_ERROR(Status);
    }
    Status = PciIoProtocolInstance->Pci.Write(PciIoProtocolInstance, EfiPciIoWidthUint8, ASMEDIA_CONFIGURATION_CONTROL_REG, 1, ((VOID *)&WriteOperation));
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_INFO, "%a - Failed to write write operation message to mailbox. Status - %r\n", __FUNCTION__, Status));
      ASSERT_EFI_ERROR(Status);
    }
    return EFI_SUCCESS;

}


STATIC EFI_STATUS AppleAsmediaReceiveMessage(IN EFI_PCI_IO_PROTOCOL *PciIoProtocolInstance, IN UINT64 *MessageToReceive) {
    UINT8 Operation;
    UINT32 MessageLow = 0;
    UINT32 MessageHigh = 0;
    CONST UINT8 ReadOperation = ASMEDIA_CONFIGURATION_CONTROL_READ_BIT;
    EFI_STATUS Status;

    for(INT32 i = 0; i < TIMEOUT_USEC; i++) {
        Status = PciIoProtocolInstance->Pci.Read(PciIoProtocolInstance, EfiPciIoWidthUint8, ASMEDIA_CONFIGURATION_CONTROL_REG, 1, ((VOID *)&Operation));
        if(!(Operation & ASMEDIA_CONFIGURATION_CONTROL_READ_BIT)) {
            break;
        }
        MicroSecondDelay(1);
    }
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_INFO, "%a - Failed to read operation message from mailbox. Status - %r\n", __FUNCTION__, Status));
      ASSERT_EFI_ERROR(Status);
    }
    if(Operation & ASMEDIA_CONFIGURATION_CONTROL_READ_BIT) {
        DEBUG((DEBUG_INFO, "%a: Mailbox read timed out\n", __FUNCTION__));
        return EFI_TIMEOUT;
    }
    Status = PciIoProtocolInstance->Pci.Read(PciIoProtocolInstance, EfiPciIoWidthUint32, ASMEDIA_CONFIGURATION_DATA_READ_REG_0, 1, ((VOID *)&MessageLow));
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_INFO, "%a - Failed to read low part of message from mailbox. Status - %r\n", __FUNCTION__, Status));
      ASSERT_EFI_ERROR(Status);
    }
    Status = PciIoProtocolInstance->Pci.Read(PciIoProtocolInstance, EfiPciIoWidthUint32, ASMEDIA_CONFIGURATION_DATA_READ_REG_1, 1, ((VOID *)&MessageHigh));
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_INFO, "%a - Failed to read low part of message from mailbox. Status - %r\n", __FUNCTION__, Status));
      ASSERT_EFI_ERROR(Status);
    }
    Status = PciIoProtocolInstance->Pci.Write(PciIoProtocolInstance, EfiPciIoWidthUint8, ASMEDIA_CONFIGURATION_CONTROL_REG, 1, ((VOID *)&ReadOperation));
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_INFO, "%a - Failed to write read operation to mailbox. Status - %r\n", __FUNCTION__, Status));
      ASSERT_EFI_ERROR(Status);
    }
    *MessageToReceive = (((UINT64)(MessageHigh) << 32) | (MessageLow));
    return EFI_SUCCESS;
}


//
// Description:
//   Gets the firmware version the XHCI controller is running.
//
// Return values:
//   EFI_NOT_STARTED if the Asmedia firmware is the ROM version
//   EFI_ALREADY_STARTED if the Asmedia firmware is loaded from disk or FV.
//   EFI_ABORTED in case an error occurred.
//
STATIC EFI_STATUS AppleAsmediaGetFirmwareVersion(IN EFI_PCI_IO_PROTOCOL *PciIoProtocolInstance, OUT UINT64 *FirmwareVersionStored) {
    UINT64 FirmwareVersion;
    EFI_STATUS Status;
    UINT64 Command;
    
    Status = AppleAsmediaSendMessage(PciIoProtocolInstance, ASMEDIA_COMMAND_GET_FIRMWARE_VERSION);
    if (EFI_ERROR(Status)) {
        return EFI_ABORTED;
    }
    Status = AppleAsmediaSendMessage(PciIoProtocolInstance, 0);
    if (EFI_ERROR(Status)) {
        return EFI_ABORTED;
    }
    Status = AppleAsmediaReceiveMessage(PciIoProtocolInstance, &Command);
    if (EFI_ERROR(Status)) {
        return EFI_ABORTED;
    }
    if (Command != ASMEDIA_COMMAND_GET_FIRMWARE_VERSION) {
        DEBUG((DEBUG_INFO, "%a: Unexpected command 0x%llx\n", __FUNCTION__, Command));
        return EFI_ABORTED;
    }
    Status = AppleAsmediaReceiveMessage(PciIoProtocolInstance, &FirmwareVersion);
    if (EFI_ERROR(Status)) {
        return EFI_ABORTED;
    }
    *FirmwareVersionStored = FirmwareVersion;
    return EFI_SUCCESS;
}

STATIC BOOLEAN AppleAsmediaCheckFirmwareIsLoaded(IN EFI_PCI_IO_PROTOCOL *PciIoProtocolInstance) {
    UINT64 FirmwareVersion = 0;
    EFI_STATUS Status;
    Status = AppleAsmediaGetFirmwareVersion(PciIoProtocolInstance, &FirmwareVersion);
    if(EFI_ERROR(Status)) {
      DEBUG((DEBUG_ERROR, "%a - Error occurred when getting firmware version\n", __FUNCTION__));
      ASSERT(FALSE);
    }
    return FirmwareVersion != ASMEDIA_ROM_FIRMWARE_REVISION;
}

STATIC EFI_STATUS AppleAsmediaWaitReset(IN EFI_PCI_IO_PROTOCOL *PciIoProtocolInstance) {
  UINT32 OperationRegOffset = 0;
  EFI_STATUS Status;
  UINT32 XhciOperationRegValue;
  CONST UINT8 SramAccessEnable = ASMEDIA_CONFIGURATION_SRAM_ACCESS_ENABLE_BIT;
  CONST UINT8 SramAccessDisable = 0;

  Status = PciIoProtocolInstance->Mem.Read(PciIoProtocolInstance, EfiPciIoWidthUint8, PCI_BAR_IDX0, XHC_CAPLENGTH_OFFSET, 1, ((VOID *)&OperationRegOffset));
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_INFO, "%a - Failed to read capability length from XHCI controller. Status - %r\n", __FUNCTION__, Status));
    return Status;
  }
  OperationRegOffset = OperationRegOffset & 0xFF;

  Status = PciIoProtocolInstance->PollMem(PciIoProtocolInstance, EfiPciIoWidthUint32, PCI_BAR_IDX0, OperationRegOffset + XHC_USBCMD_OFFSET, XHC_USBCMD_RESET, 0, 5000000, ((UINT64 *)&XhciOperationRegValue));
  
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_INFO, "%a - Timed out when polling XHCI USB command operation reg for reset operation! Status - %r\n", __FUNCTION__, Status));
    DEBUG((DEBUG_INFO, "%a - Attempting to kick the reset\n", __FUNCTION__));
    
    Status = PciIoProtocolInstance->Pci.Write(PciIoProtocolInstance, EfiPciIoWidthUint8, ASMEDIA_CONFIGURATION_SRAM_ACCESS_REG, 1, ((VOID *)&SramAccessEnable));
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_INFO, "%a - Failed to write enable SRAM access operation to mailbox. Status - %r\n", __FUNCTION__, Status));
      return Status;
    }
    Status = PciIoProtocolInstance->Pci.Write(PciIoProtocolInstance, EfiPciIoWidthUint8, ASMEDIA_CONFIGURATION_SRAM_ACCESS_REG, 1, ((VOID *)&SramAccessDisable));
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_INFO, "%a - Failed to write disable SRAM access operation to mailbox. Status - %r\n", __FUNCTION__, Status));
      return Status;
    }
    Status = PciIoProtocolInstance->PollMem(PciIoProtocolInstance, EfiPciIoWidthUint32, PCI_BAR_IDX0, OperationRegOffset + XHC_USBCMD_OFFSET, XHC_USBCMD_RESET, 0, 5000000, ((UINT64 *)&XhciOperationRegValue));
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_INFO, "%a - Reset timed out - Status - %r\n", __FUNCTION__, Status));
      return Status;
    }
  }
  return EFI_SUCCESS;
}


STATIC VOID AppleAsmediaLoadFirmware(IN EFI_PCI_IO_PROTOCOL *PciIoProtocolInstance, IN VOID *FirmwarePointer, IN UINTN FirmwareSize, OUT BOOLEAN *FirmwareUploadSuccessful) {
    UINT64 Index = 0;
    UINT64 Addr = 0;
    UINT32 Data;
    UINT16 RAddr;
    UINT64 FirmwareChunks = FirmwareSize >> 1;
    CONST UINT16 *FirmwareData = (CONST UINT16 *)FirmwarePointer;
    EFI_STATUS Status;
    CONST UINT8 SramAccessEnable = ASMEDIA_CONFIGURATION_SRAM_ACCESS_ENABLE_BIT;
    CONST UINT8 SramAccessDisable = 0;
    INT32 i = 0;

    DEBUG((DEBUG_INFO, "%a - Resetting MMIO interface of XHCI controller\n", __FUNCTION__));
    AppleAsmediaWriteRegister(PciIoProtocolInstance, ASMEDIA_MMIO_CPU_MODE_NEXT, ASMEDIA_MMIO_CPU_MODE_HALFSPEED, FALSE);
    AppleAsmediaWriteRegister(PciIoProtocolInstance, ASMEDIA_MMIO_CPU_EXEC_CONTROL, ASMEDIA_MMIO_CPU_EXEC_CONTROL_RESET, FALSE);

    Status = AppleAsmediaWaitReset(PciIoProtocolInstance);
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_INFO, "%a - Pre upload reset failed! Status - %r\n", __FUNCTION__, Status));
      ASSERT_EFI_ERROR(Status);
    }
    DEBUG((DEBUG_INFO, "%a - Halting MMIO interface of XHCI controller\n", __FUNCTION__));
    AppleAsmediaWriteRegister(PciIoProtocolInstance, ASMEDIA_MMIO_CPU_EXEC_CONTROL, ASMEDIA_MMIO_CPU_EXEC_CONTROL_HALT, FALSE);
    DEBUG((DEBUG_INFO, "%a - enabling MMIO write of firmware for upload\n", __FUNCTION__));
    AppleAsmediaWriteRegister(PciIoProtocolInstance, ASMEDIA_MMIO_CPU_MISC, ASMEDIA_MMIO_CPU_MISC_CODE_RAM_WR, TRUE);
    Status = PciIoProtocolInstance->Pci.Write(PciIoProtocolInstance, EfiPciIoWidthUint8, ASMEDIA_CONFIGURATION_SRAM_ACCESS_REG, 1, ((VOID *)&SramAccessEnable));
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_INFO, "%a - Failed to write enable SRAM access operation to mailbox. Status - %r\n", __FUNCTION__, Status));
      ASSERT_EFI_ERROR(Status);
    }
    while(Index < FirmwareChunks) {
      Data = FirmwareData[Index];
      if ((Index | ASMEDIA_FIRMWARE_UPLOAD_CHUNK_SIZE) < FirmwareChunks) {
        Data |= FirmwareData[Index | ASMEDIA_FIRMWARE_UPLOAD_CHUNK_SIZE] << 16;
      }
      Status = PciIoProtocolInstance->Pci.Write(PciIoProtocolInstance, EfiPciIoWidthUint16, ASMEDIA_CONFIGURATION_SRAM_ADDR_REG, 1, ((VOID *)&Addr));
      if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_INFO, "%a - Writing address to SRAM configuration reg failed! Status - %r\n", __FUNCTION__, Status));
        ASSERT_EFI_ERROR(Status);
      }
      Status = PciIoProtocolInstance->Mem.Write(PciIoProtocolInstance, EfiPciIoWidthUint32, PCI_BAR_IDX0, ASMEDIA_REGISTER_CODE_WRITE_DATA, 1, ((VOID *)&Data));
      if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_INFO, "%a - Writing data to XHCI controller failed! Status - %r\n", __FUNCTION__, Status));
        ASSERT_EFI_ERROR(Status);
      }
      for(i = 0; i < TIMEOUT_USEC; i++) {
        Status = PciIoProtocolInstance->Pci.Read(PciIoProtocolInstance, EfiPciIoWidthUint16, ASMEDIA_CONFIGURATION_SRAM_ADDR_REG, 1, ((VOID *)&RAddr));
        if (EFI_ERROR(Status)) {
          DEBUG((DEBUG_INFO, "%a - failed to read SRAM config addr, status %r\n", __FUNCTION__, Status));
          ASSERT_EFI_ERROR(Status);
        }
        if (RAddr != Addr) {
          break;
        }
        MicroSecondDelay(1);
      }
      if (RAddr == Addr) {
        DEBUG((DEBUG_INFO, "%a - Write of word timed out\n"));
        Status = EFI_TIMEOUT;
        return;
      }

      if (++Index & ASMEDIA_FIRMWARE_UPLOAD_CHUNK_SIZE) {
        Index += ASMEDIA_FIRMWARE_UPLOAD_CHUNK_SIZE;
      }
      Addr += 2;
    }
    Status = PciIoProtocolInstance->Pci.Write(PciIoProtocolInstance, EfiPciIoWidthUint8, ASMEDIA_CONFIGURATION_SRAM_ACCESS_REG, 1, ((VOID *)&SramAccessDisable));
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_INFO, "%a - Failed to write disable SRAM access operation to mailbox. Status - %r\n", __FUNCTION__, Status));
      ASSERT_EFI_ERROR(Status);
    }

    AppleAsmediaWriteRegister(PciIoProtocolInstance, ASMEDIA_MMIO_CPU_MISC, 0, TRUE);
    AppleAsmediaWriteRegister(PciIoProtocolInstance, ASMEDIA_MMIO_CPU_MODE_NEXT, (ASMEDIA_MMIO_CPU_MODE_RAM | ASMEDIA_MMIO_CPU_MODE_HALFSPEED), FALSE);
    AppleAsmediaWriteRegister(PciIoProtocolInstance, ASMEDIA_MMIO_CPU_EXEC_CONTROL, 0, FALSE);
    Status = AppleAsmediaWaitReset(PciIoProtocolInstance);
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_INFO, "%a - failed post upload reset, status %r\n", __FUNCTION__, Status));
      ASSERT_EFI_ERROR(Status);
    }
    *FirmwareUploadSuccessful = TRUE;
    return;
}


//
// ******************************************************
// UEFI driver binding and driver initialization section.
// ******************************************************
//


//
// Description:
//   Checks if the driver is supported. Used as the vehicle to bootstrap the USB
//   controller firmware blob. Returns unsupported afterwards to allow the normal XHCI DXE driver to do it's
//   job.
//
STATIC EFI_STATUS EFIAPI AppleBootTimeEmbeddedFirmwareDriverBindingSupported(
  IN EFI_DRIVER_BINDING_PROTOCOL *This,
  IN EFI_HANDLE Controller,
  IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath
  )
{
  EFI_STATUS Status;
  EFI_PCI_IO_PROTOCOL *PciIoProtocol;
  // EFI_PCI_IO_PROTOCOL *XhciPciIoProtocol;
  UINT32 PciID;
  BOOLEAN IsAsmediaFirmwareLoaded = FALSE;
  BOOLEAN UsbFirmwareLoadSuccessful = FALSE;
  UINTN Bus = 0;
  UINTN Device = 0;
  UINTN Function = 0;
  UINTN Segment = 0;

  Status = gBS->OpenProtocol(Controller, &gEfiPciIoProtocolGuid, (VOID **)&PciIoProtocol, 
                             This->DriverBindingHandle, Controller, EFI_OPEN_PROTOCOL_BY_DRIVER);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  Status = PciIoProtocol->Pci.Read(PciIoProtocol, EfiPciIoWidthUint32, PCI_VENDOR_ID_OFFSET, 1, &PciID);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR,
      "%a: Pci->Pci.Read() of vendor/device id failed (Status == %r)\n",
      __FUNCTION__, Status));
    goto CloseProtocolAndExit;
  }
  Status = PciIoProtocol->GetLocation(PciIoProtocol, &Segment, &Bus, &Device, &Function);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR,
      "%a: Pci->GetLocation() of bus/device/function failed (Status == %r)\n",
      __FUNCTION__, Status));
    goto CloseProtocolAndExit;
  }
  if ((PciID & PCI_VENDOR_ID_BITMASK) != PCI_VENDOR_ID_APPLE || ((PciID >> 16)) != PCI_DEVICE_ID_APPLE_SILICON_P2P_BRIDGE) {
    DEBUG((DEBUG_INFO, "%a - failed to find ASMedia XHCI controller\n", __FUNCTION__));
    DEBUG((DEBUG_INFO, "%a - Expected: 0x%04x:0x%04x, actual 0x%04x:0x%04x\n", __FUNCTION__, PCI_VENDOR_ID_ASMEDIA, PCI_DEVICE_ID_ASMEDIA_2214A, (PciID & PCI_VENDOR_ID_BITMASK), (PciID >> 16)));
    goto CloseProtocolAndExit;
  }
  DEBUG((DEBUG_INFO, "%a - PCI segment %llu, bus %llu, device %llu, function %llu\n", __FUNCTION__, Segment, Bus, Device, Function));
  //
  // We have a Apple PCI-PCI bridge at this point, check that the one we have is on the right device number for the XHCI controller.
  // (Note this is compared to a fixed PCD per SoC)
  //
  //
  // on Apple platforms, all devices on bus 0 are the PCI to PCI bridges.
  // therefore we only need to verify the device number (since no P2P bridge has sub functions)
  //
  if((UINT32)Device != FixedPcdGet32(PcdXhciPcieDeviceNumber)) {
    DEBUG((DEBUG_INFO, "%a - skipping PCI-PCI bridge as it is not the one for the XHCI controller\n", __FUNCTION__));
    goto CloseProtocolAndExit;
  }

  //
  // Now, we need to get a handle to the real XHCI controller.
  //
  UINT64 AttributeResult1 = 0;
  UINT64 AttributeResult2 = 0;
  PciIoProtocol->Attributes(PciIoProtocol, EfiPciIoAttributeOperationGet, 0, &AttributeResult1);
  PciIoProtocol->Attributes(PciIoProtocol, EfiPciIoAttributeOperationSupported, 0, &AttributeResult2);
  DEBUG((DEBUG_INFO, "%a - Attributes set 0x%llx, Attributes supported 0x%llx\n", __FUNCTION__, AttributeResult1, AttributeResult2));
  ASSERT(FALSE);

  //
  // If we're at this point, we have the right XHCI controller, proceed to upload the firmware.
  //
  IsAsmediaFirmwareLoaded = AppleAsmediaCheckFirmwareIsLoaded(PciIoProtocol);
  if (IsAsmediaFirmwareLoaded == TRUE) {
    DEBUG((DEBUG_INFO, "%a - XHCI controller firmware upload not needed - exiting\n", __FUNCTION__));
    UsbFirmwareLoadSuccessful = TRUE;
    goto CloseProtocolAndExit;
  }
  DEBUG((DEBUG_INFO, "%a - uploading XHCI controller firmware\n", __FUNCTION__));
  AppleAsmediaLoadFirmware(PciIoProtocol, NewUsbFirmwareBlobPointer, UsbFirmwareSize, &UsbFirmwareLoadSuccessful);
  //
  // Double check that the firmware we loaded is actually the one from RAM.
  //
  IsAsmediaFirmwareLoaded = AppleAsmediaCheckFirmwareIsLoaded(PciIoProtocol);
  


CloseProtocolAndExit:
  gBS->CloseProtocol(Controller, &gEfiPciIoProtocolGuid, This->DriverBindingHandle, Controller);

  //
  // if we failed, free the USB firmware buffer.
  //
  // if(UsbFirmwareLoadSuccessful != TRUE) {
  //   FreePool(NewUsbFirmwareBlobPointer);
  // }

  //
  // Returning unsupported here allows the normal XHCI driver to take over
  // without us having to drive the controller here.
  //
  return EFI_UNSUPPORTED;
}

//
// Description:
//   Starts the driver on Controller. Unused, as this driver's purpose is only to
//   kickstart the firmware on any coprocessors that require it.
//
STATIC EFI_STATUS EFIAPI AppleBootTimeEmbeddedFirmwareDriverBindingStart(
  IN EFI_DRIVER_BINDING_PROTOCOL *This,
  IN EFI_HANDLE Controller,
  IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath
  )
{
  ASSERT(FALSE);
  return EFI_INVALID_PARAMETER;
}


//
// Description:
//   Stops the driver on Controller. Unused, as this driver's purpose is only to
//   kickstart the firmware on any coprocessors that require it.
//
STATIC EFI_STATUS EFIAPI AppleBootTimeEmbeddedFirmwareDriverBindingStop(
  IN EFI_DRIVER_BINDING_PROTOCOL *This,
  IN EFI_HANDLE Controller,
  IN UINTN NumberOfChildren,
  IN EFI_HANDLE *ChildHandleBuffer
  )
{
  ASSERT(FALSE);
  return EFI_SUCCESS;
}


//
// Driver Binding Protocol Instance.
// Yes, this is necessary to upload firmware...
//
EFI_DRIVER_BINDING_PROTOCOL  gAppleBootTimeEmbeddedFirmwareBindingBinding = {
  AppleBootTimeEmbeddedFirmwareDriverBindingSupported,
  AppleBootTimeEmbeddedFirmwareDriverBindingStart,
  AppleBootTimeEmbeddedFirmwareDriverBindingStop,
  //
  // Version values of 0xfffffff0-0xffffffff are reserved for platform/OEM
  // specific drivers. Protocol instances with higher 'Version' properties
  // will be used before lower 'Version' ones. XhciDxe uses version 0x30,
  // so this driver will be called in preference, and XhciDxe will be invoked
  // after AppleBootTimeEmbeddedFirmwareDriverBindingSupported returns EFI_UNSUPPORTED.
  //
  0xfffffff0,
  NULL,
  NULL
};

EFI_STATUS
EFIAPI 
AppleBootTimeEmbeddedFirmwareHelperDxeInitialize(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
)
{
    EFI_STATUS Status;

    DEBUG((DEBUG_INFO, "%a: AppleBootTimeEmbeddedFirmwareHelperDxe started\n", __FUNCTION__));
    //
    // Load the section of the FV that contains the USB firmware blob.
    //
    Status = GetSectionFromAnyFv(&gAppleSiliconPkgEmbeddedUsbFirmwareGuid, EFI_SECTION_RAW, 0, &UsbFirmwarePointer, &UsbFirmwareSize);
    if(EFI_ERROR(Status)) {
        DEBUG((DEBUG_ERROR, "%a: loading FV embedded firmware failed, status %r, exiting\n", __FUNCTION__, Status));
        return Status;
    }
    gUsbFirmwareFound = TRUE;
    if(gUsbFirmwareFound == TRUE) {
        //
        // Allocate a new 256K buffer for the firmware and copy it in, then load the firmware.
        //
        NewUsbFirmwareBlobPointer = AllocateCopyPool(SIZE_256KB, UsbFirmwarePointer);
    }
    //
    // Register the driver binding to start the firmware load.
    //
    return EfiLibInstallDriverBinding(ImageHandle, SystemTable, &gAppleBootTimeEmbeddedFirmwareBindingBinding, NULL);
}