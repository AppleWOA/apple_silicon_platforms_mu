/**
 * Copyright (c) 2023, amarioguy (AppleWOA authors).
 * 
 * Module Name:
 *     AppleBootTimeEmbeddedFirmwareHelperDxe.c
 * 
 * Abstract:
 *     Header for helper driver for Apple Silicon platforms to load firmware blobs required for onboard PCIe or
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

#ifndef APPLE_BOOT_TIME_EMBEDDED_FIRMWARE_HELPER_XHCI_DXE_H
#define APPLE_BOOT_TIME_EMBEDDED_FIRMWARE_HELPER_XHCI_DXE_H

#include <Library/ConvenienceMacros.h>

//
// XHCI register definitions borrowed from XhciReg.h
//

//
// Capability registers offset
//
#define XHC_CAPLENGTH_OFFSET   0x00             // Capability register length offset
#define XHC_HCIVERSION_OFFSET  0x02             // Interface Version Number 02-03h
#define XHC_HCSPARAMS1_OFFSET  0x04             // Structural Parameters 1
#define XHC_HCSPARAMS2_OFFSET  0x08             // Structural Parameters 2
#define XHC_HCSPARAMS3_OFFSET  0x0c             // Structural Parameters 3
#define XHC_HCCPARAMS_OFFSET   0x10             // Capability Parameters
#define XHC_DBOFF_OFFSET       0x14             // Doorbell Offset
#define XHC_RTSOFF_OFFSET      0x18             // Runtime Register Space Offset

//
// Operational registers offset
//
#define XHC_USBCMD_OFFSET    0x0000               // USB Command Register Offset
#define XHC_USBSTS_OFFSET    0x0004               // USB Status Register Offset
#define XHC_PAGESIZE_OFFSET  0x0008               // USB Page Size Register Offset
#define XHC_DNCTRL_OFFSET    0x0014               // Device Notification Control Register Offset
#define XHC_CRCR_OFFSET      0x0018               // Command Ring Control Register Offset
#define XHC_DCBAAP_OFFSET    0x0030               // Device Context Base Address Array Pointer Register Offset
#define XHC_CONFIG_OFFSET    0x0038               // Configure Register Offset
#define XHC_PORTSC_OFFSET    0x0400               // Port Status and Control Register Offset

//
// Runtime registers offset
//
#define XHC_MFINDEX_OFFSET  0x00                // Microframe Index Register Offset
#define XHC_IMAN_OFFSET     0x20                // Interrupter X Management Register Offset
#define XHC_IMOD_OFFSET     0x24                // Interrupter X Moderation Register Offset
#define XHC_ERSTSZ_OFFSET   0x28                // Event Ring Segment Table Size Register Offset
#define XHC_ERSTBA_OFFSET   0x30                // Event Ring Segment Table Base Address Register Offset
#define XHC_ERDP_OFFSET     0x38                // Event Ring Dequeue Pointer Register Offset

//
// Debug registers offset
//
#define XHC_DC_DCCTRL  0x20

#define USBLEGSP_BIOS_SEMAPHORE  BIT16           // HC BIOS Owned Semaphore
#define USBLEGSP_OS_SEMAPHORE    BIT24           // HC OS Owned Semaphore

//
// xHCI Supported Protocol Capability
//
#define XHC_SUPPORTED_PROTOCOL_DW0_MAJOR_REVISION_USB2  0x02
#define XHC_SUPPORTED_PROTOCOL_DW0_MAJOR_REVISION_USB3  0x03
#define XHC_SUPPORTED_PROTOCOL_NAME_STRING_OFFSET       0x04
#define XHC_SUPPORTED_PROTOCOL_NAME_STRING_VALUE        0x20425355
#define XHC_SUPPORTED_PROTOCOL_DW2_OFFSET               0x08
#define XHC_SUPPORTED_PROTOCOL_PSI_OFFSET               0x10
#define XHC_SUPPORTED_PROTOCOL_USB2_HIGH_SPEED_PSIM     480
#define XHC_SUPPORTED_PROTOCOL_USB2_LOW_SPEED_PSIM      1500


//
// Register Bit Definition
//
#define XHC_USBCMD_RUN    BIT0                   // Run/Stop
#define XHC_USBCMD_RESET  BIT1                   // Host Controller Reset
#define XHC_USBCMD_INTE   BIT2                   // Interrupter Enable
#define XHC_USBCMD_HSEE   BIT3                   // Host System Error Enable


#define PCI_VENDOR_ID_ASMEDIA 0x1B21
#define PCI_VENDOR_ID_APPLE 0x106B
#define PCI_DEVICE_ID_APPLE_SILICON_P2P_BRIDGE 0x100C
#define PCI_DEVICE_ID_ASMEDIA_2214A 0x2142
#define PCI_VENDOR_ID_BITMASK 0xffff
//
// Definitions for the ASMedia PCIe XHCI device.
//

#define ASMEDIA_CONFIGURATION_CONTROL_REG 0xE0

#define ASMEDIA_CONFIGURATION_CONTROL_WRITE_BIT BIT(1)
#define ASMEDIA_CONFIGURATION_CONTROL_READ_BIT BIT(0)

#define ASMEDIA_CONFIGURATION_SRAM_ADDR_REG 0xE2
#define ASMEDIA_CONFIGURATION_SRAM_ACCESS_REG 0xEF

#define ASMEDIA_CONFIGURATION_SRAM_ACCESS_READ_BIT BIT(6)
#define ASMEDIA_CONFIGURATION_SRAM_ACCESS_ENABLE_BIT BIT(7)

#define ASMEDIA_CONFIGURATION_DATA_READ_REG_0 0xF0
#define ASMEDIA_CONFIGURATION_DATA_READ_REG_1 0xF4

#define ASMEDIA_CONFIGURATION_DATA_WRITE_REG_0 0xF8
#define ASMEDIA_CONFIGURATION_DATA_WRITE_REG_1 0xFC

#define ASMEDIA_COMMAND_GET_FIRMWARE_VERSION 0x8000060840

//
// PCIe BAR0 registers.
//
#define ASMEDIA_REGISTER_ADDR 0x3000
#define ASMEDIA_REGISTER_WRITE_DATA 0x3004
#define ASMEDIA_REGISTER_READ_DATA 0x3008
#define ASMEDIA_REGISTER_STATUS 0x3009

#define ASMEDIA_REGISTER_STATUS_BUSY BIT(7)

#define ASMEDIA_REGISTER_CODE_WRITE_DATA 0x3010
#define ASMEDIA_REGISTER_CODE_READ_DATA 0x3018
#define ASMEDIA_MMIO_CPU_MISC 0x500E

#define ASMEDIA_MMIO_CPU_MISC_CODE_RAM_WR BIT(0)

#define ASMEDIA_MMIO_CPU_MODE_NEXT 0x5040
#define ASMEDIA_MMIO_CPU_MODE_CURRENT 0x5041

#define ASMEDIA_MMIO_CPU_MODE_RAM BIT(0)
#define ASMEDIA_MMIO_CPU_MODE_HALFSPEED BIT(1)

#define ASMEDIA_MMIO_CPU_EXEC_CONTROL 0x5042

#define ASMEDIA_MMIO_CPU_EXEC_CONTROL_RESET BIT(0)
#define ASMEDIA_MMIO_CPU_EXEC_CONTROL_HALT BIT(1)

#define TIMEOUT_USEC 10000
#define RESET_TIMEOUT_USEC 500000


//
// This version of the ASMedia firmware indicates we are in the ROM firmware
// and that the controller is awaiting firmware upload by the AP.
//
#define ASMEDIA_ROM_FIRMWARE_REVISION 0x010250090816

//
// Firmware chunk size (16k chunks)
//
#define ASMEDIA_FIRMWARE_UPLOAD_CHUNK_SIZE 0x4000


#endif //APPLE_BOOT_TIME_EMBEDDED_FIRMWARE_HELPER_XHCI_DXE_H