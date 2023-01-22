/** @file
*
*  Based off ArmHvcLib.h from ArmPkg/ArmHvcLib, original copyright notice below.
*  
*  Copyright (c) 2012-2014, ARM Limited. All rights reserved.
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#ifndef APPLE_ARM_GXF_HVC_LIB_H_
#define APPLE_ARM_GXF_HVC_LIB_H_

/**
 * General structure is taken from ArmHvcLib.h
 */
typedef struct {
  UINTN    Arg0;
  UINTN    Arg1;
  UINTN    Arg2;
  UINTN    Arg3;
  UINTN    Arg4;
  UINTN    Arg5;
  UINTN    Arg6;
  UINTN    Arg7;
} APPLE_ARM_GXF_HVC_ARGS;


/**
  Trigger a GXF hypercall.

  GXF hypercalls for the most part generally follow the same rules as standard ARM HVC calls.

**/
VOID
AppleArmCallGxfHvc (
  IN OUT APPLE_ARM_GXF_HVC_ARGS  *Args
  );

#endif // APPLE_ARM_GXF_HVC_LIB_H_