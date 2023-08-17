/**
 * Copyright (c) 2023, amarioguy (AppleWOA authors).
 * 
 * Module Name:
 *     AppleUsbFunctionIoDxe.c
 * 
 * Abstract:
 *     USB Function IO driver for Apple Silicon platforms.
 * 
 * Environment:
 *     UEFI DXE (Driver Execution Environment).
 * 
 * License:
 *     SPDX-License-Identifier: BSD-2-Clause-Patent OR MIT
 * 
*/

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/HiiLib.h>
#include <Library/PrintLib.h>
#include <Library/PcdLib.h>

#include <PiDxe.h>

#include <Protocol/DriverBinding.h>
#include <Protocol/ComponentName2.h>
#include <Protocol/ComponentName.h>
#include <Protocol/UsbFunctionIo.h>

//
// TODO: see if this driver needs to be implemented for the ASMedia controller (ASM3142) in addition
// to the DWC3 controller.
//

EFI_STATUS EFIAPI AppleUsbFunctionIoDetectPort(IN EFI_USBFN_IO_PROTOCOL *This, OUT EFI_USBFN_PORT_TYPE *PortType) {
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI AppleUsbFunctionIoConfigureEnableEndpoints(IN EFI_USBFN_IO_PROTOCOL *This, OUT EFI_USB_DEVICE_INFO *DeviceInfo) {
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI AppleUsbFunctionIoGetEndpointMaxPacketSize(IN EFI_USBFN_IO_PROTOCOL *This, IN EFI_USB_ENDPOINT_TYPE EndpointType, IN EFI_USB_BUS_SPEED BusSpeed, OUT UINT16 *MaxPacketSize) {
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI AppleUsbFunctionIoGetVendorIdProductId(IN EFI_USBFN_IO_PROTOCOL *This, OUT UINT16 *Vid, OUT UINT16 *Pid) {
    *Vid = (UINT16)0x05AC; //Apple's vendor ID
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI AppleUsbFunctionIoGetDeviceInfo(IN EFI_USBFN_IO_PROTOCOL *This, IN EFI_USBFN_DEVICE_INFO_ID Id, IN OUT UINTN *BufferSize, OUT VOID *Buffer OPTIONAL) {
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI AppleUsbFunctionIoAbortTransfer(IN EFI_USBFN_IO_PROTOCOL *This, IN UINT8 EndpointIndex, IN EFI_USBFN_ENDPOINT_DIRECTION Direction) {
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI AppleUsbFunctionIoGetEndpointStallState(IN EFI_USBFN_IO_PROTOCOL *This, IN UINT8 EndpointIndex, IN EFI_USBFN_ENDPOINT_DIRECTION Direction, IN OUT BOOLEAN *State) {
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI AppleUsbFunctionIoSetEndpointStallState(IN EFI_USBFN_IO_PROTOCOL *This, IN UINT8 EndpointIndex, IN EFI_USBFN_ENDPOINT_DIRECTION Direction, IN OUT BOOLEAN *State) {
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI AppleUsbFunctionIoEventHandler(IN EFI_USBFN_IO_PROTOCOL *This, OUT EFI_USBFN_MESSAGE *Message, IN OUT UINTN *PayloadSize, OUT EFI_USBFN_MESSAGE_PAYLOAD *Payload) {
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI AppleUsbFunctionIoTransfer(IN EFI_USBFN_IO_PROTOCOL *This, IN UINT8 EndpointIndex, IN EFI_USBFN_ENDPOINT_DIRECTION, IN OUT UINTN *BufferSize, IN OUT VOID *Buffer) {
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI AppleUsbFunctionIoGetMaxTransferSize(IN EFI_USBFN_IO_PROTOCOL *This, OUT UINTN *MaxTransferSize) {
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI AppleUsbFunctionIoAllocateTransferBuffer(IN EFI_USBFN_IO_PROTOCOL *This, IN UINTN Size, OUT VOID **Buffer) {
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI AppleUsbFunctionIoFreeTransferBuffer(IN EFI_USBFN_IO_PROTOCOL *This, IN VOID *Buffer) {
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI AppleUsbFunctionIoStartController(IN EFI_USBFN_IO_PROTOCOL *This) {
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI AppleUsbFunctionIoStopController(IN EFI_USBFN_IO_PROTOCOL *This) {
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI AppleUsbFunctionIoSetEndpointPolicy(IN EFI_USBFN_IO_PROTOCOL *This, IN UINT8 EndpointIndex, IN EFI_USBFN_ENDPOINT_DIRECTION Direction, IN EFI_USBFN_POLICY_TYPE, IN UINTN BufferSize, IN VOID *Buffer) {
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI AppleUsbFunctionIoGetEndpointPolicy(IN EFI_USBFN_IO_PROTOCOL *This, IN UINT8 EndpointIndex, IN EFI_USBFN_ENDPOINT_DIRECTION Direction, IN EFI_USBFN_POLICY_TYPE PolicyType, IN OUT UINTN *BufferSize, IN OUT VOID *Buffer) {
    return EFI_SUCCESS;
}

EFI_USBFN_IO_PROTOCOL gUsbFunctionIo = {
    0x00010001, //Revision
    AppleUsbFunctionIoDetectPort,
    AppleUsbFunctionIoConfigureEnableEndpoints,
    AppleUsbFunctionIoGetEndpointMaxPacketSize,
    AppleUsbFunctionIoGetDeviceInfo,
    AppleUsbFunctionIoGetVendorIdProductId,
    AppleUsbFunctionIoAbortTransfer,
    AppleUsbFunctionIoGetEndpointStallState,
    AppleUsbFunctionIoSetEndpointStallState,
    AppleUsbFunctionIoEventHandler,
    AppleUsbFunctionIoTransfer,
    AppleUsbFunctionIoGetMaxTransferSize,
    AppleUsbFunctionIoAllocateTransferBuffer,
    AppleUsbFunctionIoFreeTransferBuffer,
    AppleUsbFunctionIoStartController,
    AppleUsbFunctionIoStopController,
    AppleUsbFunctionIoSetEndpointPolicy,
    AppleUsbFunctionIoGetEndpointPolicy
};

//
// Description:
//   Initializes the USB Function IO protocol.
//
// Return value:
//   None.
//
EFI_STATUS AppleUsbFunctionIoDxeInitialize(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) {

    EFI_STATUS Status;
    EFI_HANDLE UsbFunctionIoProtocolHandle = NULL;
    //
    // Install the USB function IO protocol.
    //
    Status = gBS->InstallMultipleProtocolInterfaces(&UsbFunctionIoProtocolHandle, &gEfiUsbFunctionIoProtocolGuid, &gUsbFunctionIo, NULL);
}