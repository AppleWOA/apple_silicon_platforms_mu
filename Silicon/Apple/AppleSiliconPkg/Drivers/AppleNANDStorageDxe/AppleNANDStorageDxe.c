/**
 * Copyright (c) 2023, amarioguy (AppleWOA authors).
 * 
 * Module Name:
 *     AppleNANDStorageDxe.c
 * 
 * Abstract:
 *     Apple NAND Storage controller driver for Apple Silicon platforms.
 * 
 * Environment:
 *     UEFI DXE (Driver Execution Environment).
 * 
 * License:
 *     SPDX-License-Identifier: BSD-2-Clause-Patent OR MIT
 * 
*/

//
// We basically need to roll our own NVMe driver here, because
// Apple devices do not have "standard" NVMe, which is normally exposed
// over PCIe, and Apple's implementation of NVMe (over the ANS2 controller)
// breaks the spec in a few ways.
//