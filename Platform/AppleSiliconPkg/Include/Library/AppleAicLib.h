/**
 * @file AppleAicLib.h
 * @author amarioguy (Arminder Singh)
 * @brief 
 * 
 * AppleAicLib C header file. Contains register defines/other things needed by the library
 * 
 * Note - all register defines are relative to AIC_V1_BASE or AIC_V2_BASE
 * 
 * @date 2022-09-05
 * 
 * @copyright Copyright (c) amarioguy (Arminder Singh), 2022.
 * 
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 * 
 */

#ifndef APPLEAIC_H_
#define APPLEAIC_H_

#include <Library/PcdLib.h>
#include <PiDxe.h>
#include <Library/ConvenienceMacros.h>

/*
 * AIC versions.
 */
typedef enum {
    APPLE_AIC_VERSION_1,
    APPLE_AIC_VERSION_2,
    APPLE_AIC_VERSION_UNKNOWN
} APPLE_AIC_VERSION;

/**
 * 
 * AIC structs, to keep register offsets and frequently used values in a single place to keep the code readable/sane.
 * 
 * Inspired by m1n1 and Linux
 * 
 */

typedef struct aic_reg_info_struct {
    //required on AICv2, AICv1 has a static value for this register.
    UINT64 EventReg;

    // target cpu reg only used on AICv1
    UINT64 TargetCpuRegOffset;
    UINT64 IrqConfigRegOffset;
    UINT64 SoftwareSetRegOffset;
    UINT64 SoftwareClearRegOffset;
    UINT64 IrqMaskSetRegOffset;
    UINT64 IrqMaskClearRegOffset;
    UINT64 HwStateRegOffset;
} AIC_REG_INFO;

typedef struct aic_info_struct {
    UINT32 NumIrqs;
    UINT32 NumCpuDies;
    UINT32 MaxIrqs;
    UINT32 MaxCpuDies;
    //used to ensure that AIC MMIO writes are applying to the correct CPU die
    UINT32 DieStride;
    UINT32 RegSize;

    AIC_REG_INFO Regs;
} AIC_INFO_STRUCT;

extern AIC_INFO_STRUCT *AicInfoStruct;


//AICv1 Registers

#define AIC_V1_HW_INFO 0x0004
//AIC_WHOAMI in m1n1/Linux sources
//on AICv1, used in CPU affinity modifications
#define AIC_V1_CPU_IDENTIFIER_REG 0x2000
#define AIC_V1_EVENT_REG 0x2004
#define AIC_V1_SEND_IPI_REG 0x2008
#define AIC_V1_ACKNOWLEDGE_IPI_REG 0x200c
//mask/clear IPIs
#define AIC_V1_SET_IPI_MASK_REG 0x2024
#define AIC_V1_CLEAR_IPI_MASK_REG 0x2028


//AIC Event Types
//which CPU die did this occur on?
#define AIC_EVENT_NUM_DIE GENMASK(31, 24)
//are we an FIQ, IRQ, or IPI?
#define AIC_EVENT_INTERRUPT_TYPE GENMASK(23, 16)
//Interrupt number
#define AIC_EVENT_IRQ_NUM GENMASK(15, 0)


//AICv1 bitmasks



//AICv2 Registers (the constant offsets)

#define AIC_V2_INFO_REG1 0x0004
#define AIC_V2_INFO_REG2 0x0008
#define AIC_V2_INFO_REG3 0x000c
#define AIC_V2_CONFIG 0x0014
#define AIC_V2_IRQ_CFG_REG 0x2000


//AICv2 bitmasks and bitfield definitions

#define AIC_V2_NUM_AND_MAX_IRQS_MASK GENMASK(15, 0)
#define AIC_V2_INFO_REG3_MAX_DIE_COUNT_BITFIELD GENMASK(27, 24)
#define AIC_V2_INFO_REG1_LAST_CPU_DIE_BITFIELD GENMASK(27, 24)
#define AIC_V2_CFG_ENABLE BIT(0)

// IRQ Mask macros

#define AIC_MASK_REG(num) (4 * ((num) >> 5))
#define AIC_MASK_BIT(num) BIT(num) & GENMASK(4, 0)

/* Function prototypes */


//Common

APPLE_AIC_VERSION EFIAPI AppleArmGetAicVersion(VOID);

/**
 * Reads the event register on the platform AIC, which serves as an interrupt ack
 * (masks it at the same time)
 * 
 * @return UINT32 - Event register value
 */
UINT32 EFIAPI AppleAicAcknowledgeInterrupt(
    IN VOID
);

/**
 * Return the implemented number of IRQs on the platform.
 * 
 * @param AicBase - AIC base address
 * @return UINT32 - number of IRQs
 */
UINT32 EFIAPI AppleAicGetNumInterrupts(
    IN UINTN AicBase
);

/**
 * Returns the maximum supported number of IRQs on the SoC.
 * 
 * @param AicBase - AIC Base Address
 * @return UINT32 - max number of IRQs supported on the SoC.
 */
UINT32 EFIAPI AppleAicGetMaxInterrupts(
    IN UINTN AicBase
);

/**
 * @brief Masks an IRQ by writing to the AIC's MASK_SET register.
 * 
 * Note that all FIQ sources are directly implemented in the cores instead of going through AIC.
 * 
 * TODO: deal with FIQs.
 * 
 * @param AicBase - AIC Base Address
 * @param Source - IRQ number 
 * 
 */
VOID EFIAPI AppleAicMaskInterrupt(
    IN UINTN AicBase,
    IN UINTN Source
);

/**
 * @brief Unmask an IRQ by writing to the AIC's MASK_SET register.
 * 
 * Note that all FIQ sources are directly implemented in the cores instead of going through AIC.
 * 
 * TODO: deal with FIQs.
 * 
 * @param AicBase - AIC Base Address
 * @param Source - IRQ number 
 * 
 */
VOID EFIAPI AppleAicUnmaskInterrupt(
    IN UINTN AicBase,
    IN UINTN Source
);


/**
 * Read interrupt state from the AIC's HW_STATE register
 * 
 * @param AicBase - AIC base address
 * @param Source - IRQ number
 * @return TRUE if enabled, FALSE if disabled.
 */
BOOLEAN EFIAPI AppleAicReadInterruptState(
    IN UINTN AicBase,
    IN UINTN Source
);




//AICv1 specific



//AICv2 specific

UINTN EFIAPI AppleAicV2ReadIpiStatusRegister(VOID);

VOID EFIAPI AppleAicV2WriteIpiStatusRegister(IN UINTN Value);

VOID EFIAPI AppleAicV2WriteIpiLocalRequestRegister(IN UINTN Value);

VOID EFIAPI AppleAicV2WriteIpiGlobalRequestRegister(IN UINTN Value);

UINTN EFIAPI AppleAicV2ReadPmcControlRegister(VOID);

VOID EFIAPI AppleAicV2WritePmcControlRegister(IN UINTN Value);

UINTN EFIAPI AppleAicV2ReadUncorePmcControlRegister(VOID);

VOID EFIAPI AppleAicV2WriteUncorePmcControlRegister(IN UINTN Value);

UINTN EFIAPI AppleAicV2ReadUncorePmcStatusRegister(VOID);

/**
 * Calculate the AIC register offsets on the platform
 * 
 * @return EFI_SUCCESS if successful, EFI_ERROR(-1) if an error occurred.
 */


#endif //APPLEAIC_H_