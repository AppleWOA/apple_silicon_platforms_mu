/**
 * Copyright (c) 2024, amarioguy (AppleWOA authors).
 * 
 * Module Name:
 *     AppleDartIoMmuDxe.c
 * 
 * Abstract:
 *     Platform specific driver for Apple silicon platforms to set up the DARTs.
 *     Note that the DARTs as of right now are being configured in bypass mode, so
 *     security of device memory acccesses is not as strong as it could be.
 * 
 * 
 * Environment:
 *     UEFI DXE (Driver Execution Environment).
 * 
 * License:
 *     SPDX-License-Identifier: (BSD-2-Clause-Patent OR MIT) AND GPL-2.0
 * 
 *     Original code basis is from the Asahi Linux project fork of u-boot, original copyright and author notices below.
 *     Copyright (C) 2021 Mark Kettenis <kettenis@openbsd.org>
*/

#ifndef APPLE_DART_IOMMU_DXE_H
#define APPLE_DART_IOMMU_DXE_H

#include <Library/ConvenienceMacros.h>

//
// Type definitions. Ported from AsahiLinux/u-boot project
//

typedef enum {
	AppleDartT8020Compatible = 0,
	AppleDartT8110Compatible
} APPLE_DART_TYPE;

typedef struct AppleDartInfoStruct {
	UINT64 BaseAddress;
	UINT64 *L1;
	UINT64 *L2;
	BOOLEAN BypassMode;
	INT32 Shift;
	PHYSICAL_ADDRESS DmaVirtAddrBase;
	PHYSICAL_ADDRESS DmaVirtAddrEnd;

	INT32 Nsid;
	INT32 Nttbr;
	INT32 SidEnableBase;
	INT32 TcrBase;
	UINT32 TcrTranslateEnable;
	UINT32 TcrBypass;
	INT32 TtbrBase;
	UINT32 TtbrIsValid;
	//
	// This is needed due to different peripherals potentially having different types of DARTs. (T8110 style and T8020 style DARTs flush the TLB differently.)
	//
	void (*TlbFlush)(VOID *DartInfoStruct);
} APPLE_DART_INFO;

typedef struct AppleDartMapping {
	PHYSICAL_ADDRESS HostAddr;
	PHYSICAL_ADDRESS DmaVirtualAddr;
	PHYSICAL_ADDRESS PhysAddress;
	unsigned long PhysicalSize;
	unsigned long Offset;
	UINTN NumBytes;
} APPLE_DART_MAPPING;


//
// Definitions taken from AsahiLinux/u-boot/drivers/iommu/apple_dart.c
//

#define DART_PARAMS2		0x0004
#define  DART_PARAMS2_BYPASS_SUPPORT	BIT(0)

#define DART_T8020_TLB_CMD		0x0020
#define  DART_T8020_TLB_CMD_FLUSH		BIT(20)
#define  DART_T8020_TLB_CMD_BUSY		BIT(2)
#define DART_T8020_TLB_SIDMASK		0x0034
#define DART_T8020_ERROR		0x0040
#define DART_T8020_ERROR_ADDR_LO	0x0050
#define DART_T8020_ERROR_ADDR_HI	0x0054
#define DART_T8020_CONFIG		0x0060
#define  DART_T8020_CONFIG_LOCK			BIT(15)
#define DART_T8020_SID_ENABLE		0x00fc
#define DART_T8020_TCR_BASE		0x0100
#define  DART_T8020_TCR_TRANSLATE_ENABLE	BIT(7)
#define  DART_T8020_TCR_BYPASS_DART		BIT(8)
#define  DART_T8020_TCR_BYPASS_DAPF		BIT(12)
#define DART_T8020_TTBR_BASE		0x0200
#define  DART_T8020_TTBR_VALID			BIT(31)

#define DART_T8110_PARAMS4		0x000c
#define  DART_T8110_PARAMS4_NSID_MASK		(0x1ff << 0)
#define DART_T8110_TLB_CMD		0x0080
#define  DART_T8110_TLB_CMD_BUSY		BIT(31)
#define  DART_T8110_TLB_CMD_FLUSH_ALL		BIT(8)
#define DART_T8110_ERROR		0x0100
#define DART_T8110_ERROR_MASK		0x0104
#define DART_T8110_ERROR_ADDR_LO	0x0170
#define DART_T8110_ERROR_ADDR_HI	0x0174
#define DART_T8110_PROTECT		0x0200
#define  DART_T8110_PROTECT_TTBR_TCR		BIT(0)
#define DART_T8110_SID_ENABLE_BASE	0x0c00
#define DART_T8110_TCR_BASE		0x1000
#define  DART_T8110_TCR_BYPASS_DAPF		BIT(2)
#define  DART_T8110_TCR_BYPASS_DART		BIT(1)
#define  DART_T8110_TCR_TRANSLATE_ENABLE	BIT(0)
#define DART_T8110_TTBR_BASE		0x1400
#define  DART_T8110_TTBR_VALID			BIT(0)

#define DART_SID_ENABLE(DartInfo, idx) \
	((DartInfo).SidEnableBase + 4 * (idx))
#define DART_TCR(DartInfo, sid)	((DartInfo).TcrBase + 4 * (sid))
#define DART_TTBR(DartInfo, sid, idx)	\
	((DartInfo).TtbrBase + 4 * (DartInfo).Nttbr * (sid) + 4 * (idx))
#define  DART_TTBR_SHIFT	12

#define DART_ALL_STREAMS(DartInfo)	((1U << (DartInfo)->Nsid) - 1)

#define DART_PAGE_SIZE		SIZE_16KB
#define DART_PAGE_MASK		(DART_PAGE_SIZE - 1)

#define DART_L1_TABLE		0x3
#define DART_L2_INVAL		0
#define DART_L2_VALID		BIT(0)
#define DART_L2_FULL_PAGE	BIT(1)
#define DART_L2_START(addr)	((((addr) & DART_PAGE_MASK) >> 2) << 52)
#define DART_L2_END(addr)	((((addr) & DART_PAGE_MASK) >> 2) << 40)



#endif //APPLE_DART_IOMMU_DXE_H