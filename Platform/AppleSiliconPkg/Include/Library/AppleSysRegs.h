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

#define APPLE_FAST_IPI_REQUEST_LOCAL_EL1 S3_5_15_0_0
#define APPLE_FAST_IPI_REQUEST_GLOBAL_EL1 S3_5_15_0_1
#define APPLE_FAST_IPI_STATUS_REGISTER_EL1 S3_5_15_1_1


//EL1 FIQ timer enablement register (not helpful when we're running in EL1 but when outside the m1n1 hypervisor will be helpful)
#define APPLE_EL1_TIMER_FIQ_ENABLE S3_5_15_1_3
#define EL1_VIRT_TIMER_ENABLE_BIT BIT(0)
#define EL1_PHYS_TIMER_ENABLE_BIT BIT(1)




#endif //APPLE_SYSREGS_H