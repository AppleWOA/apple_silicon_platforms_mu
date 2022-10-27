/**
 * AppleSysRegs.h
 * @author amarioguy (Arminder Singh)
 * 
 * Definitions of Apple implementation defined system registers.
 * 
 * Putting the register definitions in here helps avoid polluting other headers with these system registers.
 * 
 * @version 1.0
 * @date 2022-10-23
 * 
 * @copyright Copyright (c) amarioguy (Arminder Singh), 2022
 * 
 */

#ifndef APPLE_SYSREGS_H
#define APPLE_SYSREGS_H

#include <ConvenienceMacros.h>

/**
 * Fast IPI request and status registers.
 * 
 * In this case, "local" is "local to a core cluster" and
 * "global" is "global to the entire set of core clusters"
 * 
 */
#define APPLE_FAST_IPI_REQUEST_LOCAL_EL1 S3_5_C15_C0_0
#define APPLE_FAST_IPI_REQUEST_GLOBAL_EL1 S3_5_C15_C0_1
#define APPLE_FAST_IPI_STATUS_REGISTER_EL1 S3_5_C15_C1_1
//not sure if we need this, but better to have it just in case...
#define APPLE_FAST_IPI_COUNTDOWN_REG_EL1 S3_5_C15_C3_1


//EL1 FIQ timer enablement register (not helpful when we're running in EL1 but when outside the m1n1 hypervisor will be helpful)
#define APPLE_EL1_TIMER_FIQ_ENABLE S3_5_C15_C1_3
#define EL1_VIRT_TIMER_ENABLE_BIT BIT(0)
#define EL1_PHYS_TIMER_ENABLE_BIT BIT(1)


/**
 * IRQ masking register. (3, 4, 15, 10, 4)
 * 
 * Note: could eventually be moved to AICv2 ASM file if it's only needed there.
 * 
 */
#define APPLE_IRQ_FIQ_OPT_OUT_REG S3_4_C15_C10_4
#define IRQ_OPT_OUT_VALUE 0x3 // BIT(1) | BIT(0)


/**
 * Standard PMC registers/defines
 * 
 * Note: unused for now, but we need to ack it whenever the IRQ fires.
 * 
 */

#define APPLE_PMCR0_EL1 S3_1_C15_C0_0
#define APPLE_PMCR1_EL1 S3_1_C15_C1_0
#define APPLE_PMCR2_EL1 S3_1_C15_C2_0
#define APPLE_PMCR3_EL1 S3_1_C15_C3_0
#define APPLE_PMCR4_EL1 S3_1_C15_C4_0

#define APPLE_PMESR0_EL1 S3_1_C15_C5_0
#define APPLE_PMESR1_EL1 S3_1_C15_C6_0
#define APPLE_PMSR_EL1 S3_1_C15_C13_0


/**
 * Uncore PMC registers/defines
 * 
 * Note: unused for now, same story as standard PMCs, only use when IRQ firing.
 * 
 */

#define APPLE_UPMCR0_EL1 S3_7_C15_C0_4
#define APPLE_UPMSR_EL1 S3_7_C15_C6_4
#define APPLE_UPMCR0_IMODE GENMASK(18, 16)
#define APPLE_UPMCR_OFF_IMODE 0
#define APPLE_UPMCR_FIQ_IMODE 4
#define APPLE_UPMSR_IACT BIT(0)

#endif //APPLE_SYSREGS_H