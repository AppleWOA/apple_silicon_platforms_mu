//Main implementation of SEC phase
//SPDX-License-Identifier: BSD-2-Clause


#include "PrePi.h"

VOID Main(IN VOID *StackBase, IN VOID *StackSize, IN VOID *DeviceTreePtr)
{
    //init UART, note that this can be either the vUART provided by m1n1 or the true UART, choose based on EL
    SerialPortInitialize();
    //if we reach here, something has *seriously* gone wrong
    CpuDeadLoop();
}

VOID CEntryPoint(IN VOID *StackBase, IN VOID *StackSize, IN VOID *DeviceTreePtr)
{
    Main(StackBase, StackSize, DeviceTreePtr);
}