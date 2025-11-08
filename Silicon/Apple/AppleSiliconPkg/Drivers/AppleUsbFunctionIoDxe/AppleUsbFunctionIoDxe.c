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

STATIC BOOLEAN Dwc3Initialized = TRUE; // HACK: This is temporary, this needs to change when Start/Stop controller is implemented.

//
// This driver needs to be implemented differently for both the DWC3 and ASMedia controllers,
// for drivers that support both. This driver implements it for the DWC3.
// UsbFunctionIo is effectively just a protocol that implements the functions for operating the USB controller 
// it defines against.
//

//
// UsbFunctionIo protocol functions.
//

/**
  Returns information about what USB port type was attached.

  @param[in]  This              A pointer to the EFI_USBFN_IO_PROTOCOL instance.
  @param[out] PortType          Returns the USB port type.

  @retval EFI_SUCCESS           The function returned successfully.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_DEVICE_ERROR      The physical device reported an error.
  @retval EFI_NOT_READY         The physical device is busy or not ready to
                                process this request or there is no USB port
                                attached to the device.

**/
EFI_STATUS EFIAPI AppleUsbFunctionIoDetectPort(IN EFI_USBFN_IO_PROTOCOL *This, OUT EFI_USBFN_PORT_TYPE *PortType) {
    //
    // right now, only return that we are a standard downstream port.
    //
    *PortType = EfiUsbStandardDownstreamPort;
    return EFI_SUCCESS;
}

/**
  Configures endpoints based on supplied device and configuration descriptors.

  Assuming that the hardware has already been initialized, this function configures
  the endpoints using the device information supplied by DeviceInfo, activates the
  port, and starts receiving USB events.

  This function must ignore the bMaxPacketSize0field of the Standard Device Descriptor
  and the wMaxPacketSize field of the Standard Endpoint Descriptor that are made
  available through DeviceInfo.

  @param[in]  This              A pointer to the EFI_USBFN_IO_PROTOCOL instance.
  @param[out] DeviceInfo        A pointer to EFI_USBFN_DEVICE_INFO instance.

  @retval EFI_SUCCESS           The function returned successfully.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_DEVICE_ERROR      The physical device reported an error.
  @retval EFI_NOT_READY         The physical device is busy or not ready to process
                                this request.
  @retval EFI_OUT_OF_RESOURCES  The request could not be completed due to lack of
                                resources.

**/
EFI_STATUS EFIAPI AppleUsbFunctionIoConfigureEnableEndpoints(IN EFI_USBFN_IO_PROTOCOL *This, OUT EFI_USB_DEVICE_INFO *DeviceInfo) {
    
    return EFI_SUCCESS;
}

/**
  Returns the maximum packet size of the specified endpoint type for the supplied
  bus speed.

  If the BusSpeed is UsbBusSpeedUnknown, the maximum speed the underlying controller
  supports is assumed.

  This protocol currently does not support isochronous or interrupt transfers. Future
  revisions of this protocol may eventually support it.

  @param[in]  This              A pointer to the EFI_USBFN_IO_PROTOCOLinstance.
  @param[in]  EndpointType      Endpoint type as defined as EFI_USB_ENDPOINT_TYPE.
  @param[in]  BusSpeed          Bus speed as defined as EFI_USB_BUS_SPEED.
  @param[out] MaxPacketSize     The maximum packet size, in bytes, of the specified
                                endpoint type.

  @retval EFI_SUCCESS           The function returned successfully.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_DEVICE_ERROR      The physical device reported an error.
  @retval EFI_NOT_READY         The physical device is busy or not ready to process
                                this request.

**/
EFI_STATUS EFIAPI AppleUsbFunctionIoGetEndpointMaxPacketSize(IN EFI_USBFN_IO_PROTOCOL *This, IN EFI_USB_ENDPOINT_TYPE EndpointType, IN EFI_USB_BUS_SPEED BusSpeed, OUT UINT16 *MaxPacketSize) {
    //
    // right now, assume 64 byte packet size (this is because we're running at EHCI speeds in UEFI context)
    //
    *MaxPacketSize = 64;
    return EFI_SUCCESS;
}

/**
  Returns the vendor-id and product-id of the device.

  @param[in]  This              A pointer to the EFI_USBFN_IO_PROTOCOL instance.
  @param[out] Vid               Returned vendor-id of the device.
  @param[out] Pid               Returned product-id of the device.

  @retval EFI_SUCCESS           The function returned successfully.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_NOT_FOUND         Unable to return the vendor-id or the product-id.

**/
EFI_STATUS EFIAPI AppleUsbFunctionIoGetVendorIdProductId(IN EFI_USBFN_IO_PROTOCOL *This, OUT UINT16 *Vid, OUT UINT16 *Pid) {
    *Vid = (UINT16)0x05AC; //Apple's vendor ID
    //
    // Spoof a M2 Pro MacBook Pro for now, but note that this will need to change to machine-specific identification.
    //
    *Pid = (UINT16)0x1905; // The product ID for MacBook Pro

    return EFI_SUCCESS;
}
/**
  Returns device specific information based on the supplied identifier as a Unicode string.

  If the supplied Buffer isn't large enough, or is NULL, the method fails with
  EFI_BUFFER_TOO_SMALL and the required size is returned through BufferSize. All returned
  strings are in Unicode format.

  An Id of EfiUsbDeviceInfoUnknown is treated as an invalid parameter.

  @param[in]  This              A pointer to the EFI_USBFN_IO_PROTOCOLinstance.
  @param[in]  Id                The requested information id.


  @param[in]  BufferSize        On input, the size of the Buffer in bytes. On output, the
                                amount of data returned in Buffer in bytes.
  @param[out] Buffer            A pointer to a buffer to returnthe requested information
                                as a Unicode string.

  @retval EFI_SUCCESS           The function returned successfully.
  @retval EFI_INVALID_PARAMETER One or more of the following conditions is TRUE:
                                BufferSize is NULL.
                                *BufferSize is not 0 and Buffer is NULL.
                                Id in invalid.
  @retval EFI_DEVICE_ERROR      The physical device reported an error.
  @retval EFI_BUFFER_TOO_SMALL  The buffer is too small to hold the buffer.
                                *BufferSize has been updated with the size needed to hold the request string.

**/
EFI_STATUS EFIAPI AppleUsbFunctionIoGetDeviceInfo(IN EFI_USBFN_IO_PROTOCOL *This, IN EFI_USBFN_DEVICE_INFO_ID Id, IN OUT UINTN *BufferSize, OUT VOID *Buffer OPTIONAL) {
    const UINTN MinBufferSize = 25; //"Generic Apple USB Device" with null termination at the end.
    EFI_STATUS Status = EFI_SUCCESS;
    if((Buffer == NULL) || (BufferSize == 0) || (BufferSize < MinBufferSize)) {
        //
        // we have no buffer, return the required buffer size.
        //
        DEBUG((DEBUG_ERROR, "%a - Buffer too small, expected buffer of size %d\n", MinBufferSize));
        Status = EFI_BUFFER_TOO_SMALL;
        return Status;
    }
    Status = AsciiStrnCpyS(Buffer, BufferSize, "Generic Apple USB Device", MinBufferSize);

    return Status;
}


/**
  Aborts the transfer on the specified endpoint.

  This function should fail with EFI_INVALID_PARAMETER if the specified direction
  is incorrect for the endpoint.

  @param[in]  This              A pointer to the EFI_USBFN_IO_PROTOCOL instance.
  @param[in]  EndpointIndex     Indicates the endpoint on which the ongoing transfer
                                needs to be canceled.
  @param[in]  Direction         Direction of the endpoint.

  @retval EFI_SUCCESS           The function returned successfully.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_DEVICE_ERROR      The physical device reported an error.
  @retval EFI_NOT_READY         The physical device is busy or not ready to process
                                this request.

**/
EFI_STATUS EFIAPI AppleUsbFunctionIoAbortTransfer(IN EFI_USBFN_IO_PROTOCOL *This, IN UINT8 EndpointIndex, IN EFI_USBFN_ENDPOINT_DIRECTION Direction) {
    //
    // Note that the command being sent is the ENDTRANSFER command.
    //
    
    return EFI_SUCCESS;
}

/**
  Returns the stall state on the specified endpoint.

  This function should fail with EFI_INVALID_PARAMETER if the specified direction
  is incorrect for the endpoint.

  @param[in]      This          A pointer to the EFI_USBFN_IO_PROTOCOL instance.
  @param[in]      EndpointIndex Indicates the endpoint.
  @param[in]      Direction     Direction of the endpoint.
  @param[in, out] State         Boolean, true value indicates that the endpoint
                                is in a stalled state, false otherwise.

  @retval EFI_SUCCESS           The function returned successfully.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_DEVICE_ERROR      The physical device reported an error.
  @retval EFI_NOT_READY         The physical device is busy or not ready to process
                                this request.

**/
EFI_STATUS EFIAPI AppleUsbFunctionIoGetEndpointStallState(IN EFI_USBFN_IO_PROTOCOL *This, IN UINT8 EndpointIndex, IN EFI_USBFN_ENDPOINT_DIRECTION Direction, IN OUT BOOLEAN *State) {
    return EFI_SUCCESS;
}

/**
  Sets or clears the stall state on the specified endpoint.

  This function should fail with EFI_INVALID_PARAMETER if the specified direction
  is incorrect for the endpoint.

  @param[in]  This              A pointer to the EFI_USBFN_IO_PROTOCOL instance.
  @param[in]  EndpointIndex     Indicates the endpoint.
  @param[in]  Direction         Direction of the endpoint.
  @param[in]  State             Requested stall state on the specified endpoint.
                                True value causes the endpoint to stall; false
                                value clears an existing stall.

  @retval EFI_SUCCESS           The function returned successfully.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_DEVICE_ERROR      The physical device reported an error.
  @retval EFI_NOT_READY         The physical device is busy or not ready to process
                                this request.

**/
EFI_STATUS EFIAPI AppleUsbFunctionIoSetEndpointStallState(IN EFI_USBFN_IO_PROTOCOL *This, IN UINT8 EndpointIndex, IN EFI_USBFN_ENDPOINT_DIRECTION Direction, IN OUT BOOLEAN *State) {
    return EFI_SUCCESS;
}

/**
  This function is called repeatedly to get information on USB bus states,
  receive-completion and transmit-completion events on the endpoints, and
  notification on setup packet on endpoint 0.

  A class driver must call EFI_USBFN_IO_PROTOCOL.EventHandler()repeatedly
  to receive updates on the transfer status and number of bytes transferred
  on various endpoints.

  @param[in]      This          A pointer to the EFI_USBFN_IO_PROTOCOL instance.
  @param[out]     Message       Indicates the event that initiated this notification.
  @param[in, out] PayloadSize   On input, the size of the memory pointed by
                                Payload. On output, the amount ofdata returned
                                in Payload.
  @param[out]     Payload       A pointer to EFI_USBFN_MESSAGE_PAYLOAD instance
                                to return additional payload for current message.

  @retval EFI_SUCCESS           The function returned successfully.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_DEVICE_ERROR      The physical device reported an error.
  @retval EFI_NOT_READY         The physical device is busy or not ready to process
                                this request.
  @retval EFI_BUFFER_TOO_SMALL  The Supplied buffer is not large enough to hold
                                the message payload.

**/
EFI_STATUS EFIAPI AppleUsbFunctionIoEventHandler(IN EFI_USBFN_IO_PROTOCOL *This, OUT EFI_USBFN_MESSAGE *Message, IN OUT UINTN *PayloadSize, OUT EFI_USBFN_MESSAGE_PAYLOAD *Payload) {
    return EFI_SUCCESS;
}

/**
  This function handles transferring data to or from the host on the specified
  endpoint, depending on the direction specified.

  A class driver must call EFI_USBFN_IO_PROTOCOL.EventHandler() repeatedly to
  receive updates on the transfer status and the number of bytes transferred on
  various endpoints. Upon an update of the transfer status, the Buffer field of
  the EFI_USBFN_TRANSFER_RESULT structure (as described in the function description
  for EFI_USBFN_IO_PROTOCOL.EventHandler()) must be initialized with the Buffer
  pointer that was supplied to this method.

  The overview of the call sequence is illustrated in the Figure 54.

  This function should fail with EFI_INVALID_PARAMETER if the specified direction
  is incorrect for the endpoint.

  @param[in]      This          A pointer to the EFI_USBFN_IO_PROTOCOL instance.
  @param[in]      EndpointIndex Indicates the endpoint on which TX or RX transfer
                                needs to take place.
  @param[in]      Direction     Direction of the endpoint.
  @param[in, out] BufferSize    If Direction is EfiUsbEndpointDirectionDeviceRx:
                                  On input, the size of the Bufferin bytes.
                                  On output, the amount of data returned in Buffer
                                  in bytes.
                                If Direction is EfiUsbEndpointDirectionDeviceTx:
                                  On input, the size of the Bufferin bytes.
                                  On output, the amount of data transmitted in bytes.
  @param[in, out] Buffer        If Direction is EfiUsbEndpointDirectionDeviceRx:
                                  The Buffer to return the received data.
                                If Directionis EfiUsbEndpointDirectionDeviceTx:
                                  The Buffer that contains the data to be transmitted.

  @retval EFI_SUCCESS           The function returned successfully.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_DEVICE_ERROR      The physical device reported an error.
  @retval EFI_NOT_READY         The physical device is busy or not ready to process
                                this request.

**/
EFI_STATUS EFIAPI AppleUsbFunctionIoTransfer(IN EFI_USBFN_IO_PROTOCOL *This, IN UINT8 EndpointIndex, IN EFI_USBFN_ENDPOINT_DIRECTION, IN OUT UINTN *BufferSize, IN OUT VOID *Buffer) {
    return EFI_SUCCESS;
}

/**
  Returns the maximum supported transfer size.

  Returns the maximum number of bytes that the underlying controller can accommodate
  in a single transfer.

  @param[in]  This              A pointer to the EFI_USBFN_IO_PROTOCOL instance.
  @param[out] MaxTransferSize   The maximum supported transfer size, in bytes.

  @retval EFI_SUCCESS           The function returned successfully.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_DEVICE_ERROR      The physical device reported an error.
  @retval EFI_NOT_READY         The physical device is busy or not ready to process
                                this request.

**/
EFI_STATUS EFIAPI AppleUsbFunctionIoGetMaxTransferSize(IN EFI_USBFN_IO_PROTOCOL *This, OUT UINTN *MaxTransferSize) {
    return EFI_SUCCESS;
}
/**
  Allocates a transfer buffer of the specified sizethat satisfies the controller
  requirements.

  The AllocateTransferBuffer() function allocates a memory region of Size bytes and
  returns the address of the allocated memory that satisfies the underlying controller
  requirements in the location referenced by Buffer.

  The allocated transfer buffer must be freed using a matching call to
  EFI_USBFN_IO_PROTOCOL.FreeTransferBuffer()function.

  @param[in]  This              A pointer to the EFI_USBFN_IO_PROTOCOL instance.
  @param[in]  Size              The number of bytes to allocate for the transfer buffer.
  @param[out] Buffer            A pointer to a pointer to the allocated buffer if the
                                call succeeds; undefined otherwise.

  @retval EFI_SUCCESS           The function returned successfully.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_OUT_OF_RESOURCES  The requested transfer buffer could not be allocated.

**/
EFI_STATUS EFIAPI AppleUsbFunctionIoAllocateTransferBuffer(IN EFI_USBFN_IO_PROTOCOL *This, IN UINTN Size, OUT VOID **Buffer) {

    return EFI_SUCCESS;
}
/**
  Deallocates the memory allocated for the transfer buffer by the
  EFI_USBFN_IO_PROTOCOL.AllocateTransferBuffer() function.

  The EFI_USBFN_IO_PROTOCOL.FreeTransferBuffer() function deallocates the
  memory specified by Buffer. The Buffer that is freed must have been allocated
  by EFI_USBFN_IO_PROTOCOL.AllocateTransferBuffer().

  @param[in]  This              A pointer to the EFI_USBFN_IO_PROTOCOL instance.
  @param[in]  Buffer            A pointer to the transfer buffer to deallocate.

  @retval EFI_SUCCESS           The function returned successfully.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.

**/
EFI_STATUS EFIAPI AppleUsbFunctionIoFreeTransferBuffer(IN EFI_USBFN_IO_PROTOCOL *This, IN VOID *Buffer) {
    return EFI_SUCCESS;
}
/**
  This function supplies power to the USB controller if needed and initializes
  the hardware and the internal data structures. The port must not be activated
  by this function.

  @param[in]  This              A pointer to the EFI_USBFN_IO_PROTOCOL instance.

  @retval EFI_SUCCESS           The function returned successfully.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_DEVICE_ERROR      The physical device reported an error.

**/
EFI_STATUS EFIAPI AppleUsbFunctionIoStartController(IN EFI_USBFN_IO_PROTOCOL *This) {
    //
    // right now, we are not starting the controller.
    //
    return EFI_SUCCESS;
}
/**
  This function stops the USB hardware device.

  @param[in]  This              A pointer to the EFI_USBFN_IO_PROTOCOL instance.

  @retval EFI_SUCCESS           The function returned successfully.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_DEVICE_ERROR      The physical device reported an error.

**/
EFI_STATUS EFIAPI AppleUsbFunctionIoStopController(IN EFI_USBFN_IO_PROTOCOL *This) {
    //
    // right now, we are not stopping the controller.
    //
    return EFI_SUCCESS;
}
/**
  This function sets the configuration policy for the specified non-control
  endpoint.

  This function can only be called before EFI_USBFN_IO_PROTOCOL.StartController()
  or after EFI_USBFN_IO_PROTOCOL.StopController() has been called.

  @param[in]  This              A pointer to the EFI_USBFN_IO_PROTOCOL instance.
  @param[in]  EndpointIndex     Indicates the non-control endpoint for which the
                                policy needs to be set.
  @param[in]  Direction         Direction of the endpoint.
  @param[in]  PolicyType        Policy type the user is trying to set for the
                                specified non-control endpoint.
  @param[in]  BufferSize        The size of the Bufferin bytes.
  @param[in]  Buffer            The new value for the policy parameter that
                                PolicyType specifies.

  @retval EFI_SUCCESS           The function returned successfully.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_DEVICE_ERROR      The physical device reported an error.
  @retval EFI_UNSUPPORTED       Changing this policy value is not supported.

**/
EFI_STATUS EFIAPI AppleUsbFunctionIoSetEndpointPolicy(IN EFI_USBFN_IO_PROTOCOL *This, IN UINT8 EndpointIndex, IN EFI_USBFN_ENDPOINT_DIRECTION Direction, IN EFI_USBFN_POLICY_TYPE, IN UINTN BufferSize, IN VOID *Buffer) {
    return EFI_SUCCESS;
}
/**
  This function gets the configuration policy for the specified non-control
  endpoint.

  This function can only be called after EFI_USBFN_IO_PROTOCOL.StartController()
  or before EFI_USBFN_IO_PROTOCOL.StopController() has been called.

  @param[in]      This          A pointer to the EFI_USBFN_IO_PROTOCOL instance.
  @param[in]      EndpointIndex Indicates the non-control endpoint for which the
                                policy needs to be set.
  @param[in]      Direction     Direction of the endpoint.
  @param[in]      PolicyType    Policy type the user is trying to retrieve for
                                the specified non-control endpoint.
  @param[in, out] BufferSize    On input, the size of Bufferin bytes. On output,
                                the amount of data returned in Bufferin bytes.
  @param[in, out] Buffer        A pointer to a buffer to return requested endpoint
                                policy value.

  @retval EFI_SUCCESS           The function returned successfully.
  @retval EFI_INVALID_PARAMETER A parameter is invalid.
  @retval EFI_DEVICE_ERROR      The specified policy value is not supported.
  @retval EFI_BUFFER_TOO_SMALL  Supplied buffer is not large enough to hold requested
                                policy value.

**/
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