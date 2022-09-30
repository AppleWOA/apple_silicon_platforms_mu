/**
 * @file AppleUartSerialPortLib.h
 * 
 * 
 * @author amarioguy (Arminder Singh)
 * 
 * Register and macro defines for the UART devices on Apple Silicon Devices.
 * These are the same for the physical UART and the vUART.
 * Note that all defines are relative to UART_BASE.
 * 
 * @version 1.0
 * 
 * @copyright Copyright (c) amarioguy (Arminder Singh) 2022.
 * 
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 */

#ifndef APPLE_UART_SERIAL_PORT_H_
#define APPLE_UART_SERIAL_PORT_H_

#include <Library/PcdLib.h>
#include <PiPei.h>

//UART clock freq

#define UART_CLOCK 24000000


/* UART Register Section */

//UART configuration registers
//it seems like LCON and FCON are not used on Apple platforms?
#define UART_LCON 0x000
#define UART_CONFIG 0x004
#define UART_FCON 0x008

//UART status registers
#define UART_TRANSFER_STATUS 0x010

//UART TX/RX registers
#define UART_TX_BYTE 0x020
#define UART_RX_BYTE 0x024

#define UART_BAUD_RATE_CONFIG 0x028

#define UART_FRACTIONAL_VALUE 0x02c

/* UART Configuration Macros */

#define UART_CONFIG_TX_THRESHOLD_ENABLE (1 << 13)
#define UART_CONFIG_RX_THRESHOLD_ENABLE (1 << 12)
#define UART_CONFIG_TX_MODE_MASK 0xC // bitmask 0b1100
#define UART_CONFIG_RX_MODE_MASK 0x3 // bitmask 0b0011


/* UART Status Macros */

#define UART_TRANSFER_STATUS_RXTO (1 << 9)
#define UART_TRANSFER_STATUS_TX_THRESHOLD (1 << 5)
#define UART_TRANSFER_STATUS_RX_THRESHOLD (1 << 4)
#define UART_TRANSFER_STATUS_TXE (1 << 2)
#define UART_TRANSFER_STATUS_TXBE (1 << 1)
#define UART_TRANSFER_STATUS_RXD (1 << 0)

#define UART_FSTATUS_TX_FULL (1 << 9)
#define UART_FSTATUS_RX_FULL (1 << 8)
#define UART_FSTATUS_TX_CNT 0xF0 // bitmask 0b11110000
#define UART_FSTATUS_RX_CNT 0xF // bitmask 0b00001111

UINT32 AppleSerialPortCalculateBaudRateConfig(VOID);

UINTN SerialPortFlush(VOID);

#endif //APPLE_UART_SERIAL_PORT_H_