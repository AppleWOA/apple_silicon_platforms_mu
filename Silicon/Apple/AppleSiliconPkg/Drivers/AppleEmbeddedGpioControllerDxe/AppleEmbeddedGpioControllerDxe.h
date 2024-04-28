/**
 * Copyright (c) 2023, amarioguy (AppleWOA authors).
 * 
 * Module Name:
 *     AppleEmbeddedGpioControllerDxe.h
 * 
 * Abstract:
 *     Platform specific driver for Apple silicon platforms to control the pin/GPIO controller.
 *     Based on the PL061 ARM GPIO DXE driver.
 * 
 * Environment:
 *     UEFI DXE (Driver Execution Environment).
 * 
 * License:
 *     SPDX-License-Identifier: (BSD-2-Clause-Patent OR MIT) AND GPL-2.0
 * 
 *     Some code understanding is borrowed from the Asahi Linux project, copyright The Asahi Linux Contributors.
 * 
*/

#include <Library/ConvenienceMacros.h>

#define GPIO_REG(x) (x << 2)
#define GPIOx_REG_DATA BIT(0)
#define GPIOx_MODE GENMASK(3, 1)
#define GPIOx_OUT_MODE 1
#define GPIOx_IN_MODE_IRQ_HI 2
#define GPIOx_IN_MODE_IRQ_LO 3
#define GPIOx_IN_MODE_IRQ_UP 4
#define GPIOx_IN_MODE_IRQ_DN 5
#define GPIOx_IN_MODE_IRQ_ANY 6
#define GPIOx_IN_MODE_IRQ_OFF 7
#define GPIOx_PERIPHERAL GENMASK(6, 5)
#define GPIOx_PULL GENMASK(8, 7)
#define GPIOx_PULL_OFF 0
#define GPIOx_PULL_DOWN 1
#define GPIOx_PULL_UP_STRONG 2
#define GPIOx_PULL_UP 3
#define GPIOx_INPUT_ENABLE BIT(9)





