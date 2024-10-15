/**
 * Copyright (c) 2024, amarioguy (AppleWOA authors).
 * 
 * Module Name:
 *     AppleUsbTypeCBringupDxe.c
 * 
 * Abstract:
 *     Platform specific driver for Apple silicon platforms to bring up the USB-C ports.
 * 
 * Environment:
 *     UEFI DXE (Driver Execution Environment).
 * 
 * License:
 *     SPDX-License-Identifier: (BSD-2-Clause-Patent OR MIT) AND GPL-2.0
 * 
 *     Original code basis is from the Asahi Linux u-boot project, original copyright and author notices below.
 *     Copyright (C) 2022 Mark Kettenis <kettenis@openbsd.org>
 *     Copyright (C) The Asahi Linux Contributors.
 *     
 *     Parts of DWC3 bringup code brought in from edk2-platforms, original copyright notice below.
 *     Copyright 2017, 2020 NXP
*/

#include <Library/ConvenienceMacros.h>

//
// DWC3 constants, definitions taken from edk2-platforms/Silicon/NXP/Drivers/UsbHcdInitDxe/UsbHcd.h
//

#define DWC3_GSNPSID_MASK                      0xffff0000
#define DWC3_SYNOPSYS_ID                       0x55330000
#define DWC3_SYNOPSYS_ALT_ID                   0x33310000
#define DWC3_RELEASE_MASK                      0xffff
#define DWC3_REG_OFFSET                        0xC100
#define DWC3_RELEASE_190a                      0x190a


//
// DWC3 global control register constants and definitions, from the same file
//

#define DWC3_GCTL_U2RSTECN                     BIT16
#define DWC3_GCTL_PRTCAPDIR(N)                 ((N) << 12)
#define DWC3_GCTL_PRTCAP_HOST                  1
#define DWC3_GCTL_PRTCAP_OTG                   3
#define DWC3_GCTL_CORESOFTRESET                BIT11
#define DWC3_GCTL_SCALEDOWN(N)                 ((N) << 4)
#define DWC3_GCTL_SCALEDOWN_MASK               DWC3_GCTL_SCALEDOWN(3)
#define DWC3_GCTL_DISSCRAMBLE                  BIT3
#define DWC3_GCTL_DSBLCLKGTNG                  BIT0

//
// DWC3 global hwparams1 register
//

#define DWC3_GHWPARAMS1_EN_PWROPT(N)           (((N) & (3 << 24)) >> 24)
#define DWC3_GHWPARAMS1_EN_PWROPT_CLK          1

//
// DWC3 global USB2 PHY configuration register
//

#define DWC3_GUSB2PHYCFG_PHYSOFTRST            BIT31

//
// DWC3 global USB3 pipe control register
//

#define DWC3_GUSB3PIPECTL_PHYSOFTRST           BIT31

//
// DWC3 global frame length adjustment register
//

#define GFLADJ_30MHZ_REG_SEL                   BIT7
#define GFLADJ_30MHZ(N)                        ((N) & 0x3f)
#define GFLADJ_30MHZ_DEFAULT                   0x20

//
// Unsure what these are for, adding just in case
//
#define USB3_ENABLE_BEAT_BURST                 0xF
#define USB3_ENABLE_BEAT_BURST_MASK            0xFF
#define USB3_SET_BEAT_BURST_LIMIT              0xF00


//
// DWC3 Offsets. Currently we're using the struct approach that U-Boot uses because
// 
//

typedef struct {
  UINT32 GEvntAdrLo;
  UINT32 GEvntAdrHi;
  UINT32 GEvntSiz;
  UINT32 GEvntCount;
} G_EVENT_BUFFER;

typedef struct {
  UINT32 DDepCmdPar2;
  UINT32 DDepCmdPar1;
  UINT32 DDepCmdPar0;
  UINT32 DDepCmd;
} D_PHYSICAL_EP;

typedef struct {
  UINT32 GSBusCfg0;
  UINT32 GSBusCfg1;
  UINT32 GTxThrCfg;
  UINT32 GRxThrCfg;
  UINT32 GCtl;
  UINT32 Res1;
  UINT32 GSts;
  UINT32 Res2;
  UINT32 GSnpsId;
  UINT32 GGpio;
  UINT32 GUid;
  UINT32 GUctl;
  UINT64 GBusErrAddr;
  UINT64 GPrtbImap;
  UINT32 GHwParams0;
  UINT32 GHwParams1;
  UINT32 GHwParams2;
  UINT32 GHwParams3;
  UINT32 GHwParams4;
  UINT32 GHwParams5;
  UINT32 GHwParams6;
  UINT32 GHwParams7;
  UINT32 GDbgFifoSpace;
  UINT32 GDbgLtssm;
  UINT32 GDbgLnmcc;
  UINT32 GDbgBmu;
  UINT32 GDbgLspMux;
  UINT32 GDbgLsp;
  UINT32 GDbgEpInfo0;
  UINT32 GDbgEpInfo1;
  UINT64 GPrtbImapHs;
  UINT64 GPrtbImapFs;
  UINT32 Res3[28];
  UINT32 GUsb2PhyCfg[16];
  UINT32 GUsb2I2cCtl[16];
  UINT32 GUsb2PhyAcc[16];
  UINT32 GUsb3PipeCtl[16];
  UINT32 GTxFifoSiz[32];
  UINT32 GRxFifoSiz[32];
  G_EVENT_BUFFER GEvntBuf[32];
  UINT32 GHwParams8;
  UINT32 Res4[11];
  UINT32 GFLAdj;
  UINT32 Res5[51];
  UINT32 DCfg;
  UINT32 DCtl;
  UINT32 DEvten;
  UINT32 DSts;
  UINT32 DGCmdPar;
  UINT32 DGCmd;
  UINT32 Res6[2];
  UINT32 DAlepena;
  UINT32 Res7[55];
  D_PHYSICAL_EP DPhyEpCmd[32];
  UINT32 Res8[128];
  UINT32 OCfg;
  UINT32 OCtl;
  UINT32 OEvt;
  UINT32 OEvtEn;
  UINT32 OSts;
  UINT32 Res9[3];
  UINT32 AdpCfg;
  UINT32 AdpCtl;
  UINT32 AdpEvt;
  UINT32 AdpEvten;
  UINT32 BcCfg;
  UINT32 Res10;
  UINT32 BcEvt;
  UINT32 BcEvten;
} DWC3_CONTROLLER;

