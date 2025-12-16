/** @file
*
*  Copyright (c) 2018 - 2022, Arm Limited. All rights reserved.
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#ifndef __SGI_ACPI_HEADER__
#define __SGI_ACPI_HEADER__

#include <IndustryStandard/Acpi.h>


// Cache type identifier used to calculate unique cache ID for PPTT
typedef enum {
  L1DataCache = 1,
  L1InstructionCache,
  L2Cache,
  L3Cache,
} RD_PPTT_CACHE_TYPE;

//
// PPTT processor structure flags for different SoC components as defined in
// ACPI 6.4 specification
//

// Processor structure flags for SoC package
#define PPTT_PROCESSOR_PACKAGE_FLAGS                                           \
  {                                                                            \
    EFI_ACPI_6_4_PPTT_PACKAGE_PHYSICAL,                                        \
    EFI_ACPI_6_4_PPTT_PROCESSOR_ID_INVALID,                                    \
    EFI_ACPI_6_4_PPTT_PROCESSOR_IS_NOT_THREAD,                                 \
    EFI_ACPI_6_4_PPTT_NODE_IS_NOT_LEAF,                                        \
    EFI_ACPI_6_4_PPTT_IMPLEMENTATION_IDENTICAL                                 \
  }

// Processor structure flags for cluster
#define PPTT_PROCESSOR_CLUSTER_FLAGS                                           \
  {                                                                            \
    EFI_ACPI_6_4_PPTT_PACKAGE_NOT_PHYSICAL,                                    \
    EFI_ACPI_6_4_PPTT_PROCESSOR_ID_VALID,                                      \
    EFI_ACPI_6_4_PPTT_PROCESSOR_IS_NOT_THREAD,                                 \
    EFI_ACPI_6_4_PPTT_NODE_IS_NOT_LEAF,                                        \
    EFI_ACPI_6_4_PPTT_IMPLEMENTATION_IDENTICAL                                 \
  }

// Processor structure flags for cluster with multi-thread core
#define PPTT_PROCESSOR_CLUSTER_THREADED_FLAGS                                  \
  {                                                                            \
    EFI_ACPI_6_4_PPTT_PACKAGE_NOT_PHYSICAL,                                    \
    EFI_ACPI_6_4_PPTT_PROCESSOR_ID_INVALID,                                    \
    EFI_ACPI_6_4_PPTT_PROCESSOR_IS_NOT_THREAD,                                 \
    EFI_ACPI_6_4_PPTT_NODE_IS_NOT_LEAF,                                        \
    EFI_ACPI_6_4_PPTT_IMPLEMENTATION_IDENTICAL                                 \
  }

// Processor structure flags for single-thread core
#define PPTT_PROCESSOR_CORE_FLAGS                                              \
  {                                                                            \
    EFI_ACPI_6_4_PPTT_PACKAGE_NOT_PHYSICAL,                                    \
    EFI_ACPI_6_4_PPTT_PROCESSOR_ID_VALID,                                      \
    EFI_ACPI_6_4_PPTT_PROCESSOR_IS_NOT_THREAD,                                 \
    EFI_ACPI_6_4_PPTT_NODE_IS_LEAF                                             \
  }

// Processor structure flags for multi-thread core
#define PPTT_PROCESSOR_CORE_THREADED_FLAGS                                     \
  {                                                                            \
    EFI_ACPI_6_4_PPTT_PACKAGE_NOT_PHYSICAL,                                    \
    EFI_ACPI_6_4_PPTT_PROCESSOR_ID_INVALID,                                    \
    EFI_ACPI_6_4_PPTT_PROCESSOR_IS_NOT_THREAD,                                 \
    EFI_ACPI_6_4_PPTT_NODE_IS_NOT_LEAF,                                        \
    EFI_ACPI_6_4_PPTT_IMPLEMENTATION_IDENTICAL                                 \
  }

// Processor structure flags for CPU thread
#define PPTT_PROCESSOR_THREAD_FLAGS                                            \
  {                                                                            \
    EFI_ACPI_6_4_PPTT_PACKAGE_NOT_PHYSICAL,                                    \
    EFI_ACPI_6_4_PPTT_PROCESSOR_ID_VALID,                                      \
    EFI_ACPI_6_4_PPTT_PROCESSOR_IS_THREAD,                                     \
    EFI_ACPI_6_4_PPTT_NODE_IS_LEAF                                             \
  }

// PPTT cache structure flags as defined in ACPI 6.4 Specification
#define PPTT_CACHE_STRUCTURE_FLAGS                                             \
  {                                                                            \
    EFI_ACPI_6_4_PPTT_CACHE_SIZE_VALID,                                        \
    EFI_ACPI_6_4_PPTT_NUMBER_OF_SETS_VALID,                                    \
    EFI_ACPI_6_4_PPTT_ASSOCIATIVITY_VALID,                                     \
    EFI_ACPI_6_4_PPTT_ALLOCATION_TYPE_VALID,                                   \
    EFI_ACPI_6_4_PPTT_CACHE_TYPE_VALID,                                        \
    EFI_ACPI_6_4_PPTT_WRITE_POLICY_VALID,                                      \
    EFI_ACPI_6_4_PPTT_LINE_SIZE_VALID,                                         \
    EFI_ACPI_6_4_PPTT_CACHE_ID_VALID                                           \
  }

// PPTT cache attributes for data cache
#define PPTT_DATA_CACHE_ATTR                                                   \
  {                                                                            \
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_ALLOCATION_READ_WRITE,                       \
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_CACHE_TYPE_DATA,                             \
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_WRITE_POLICY_WRITE_BACK                      \
  }

// PPTT cache attributes for instruction cache
#define PPTT_INST_CACHE_ATTR                                                   \
  {                                                                            \
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_ALLOCATION_READ,                             \
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_CACHE_TYPE_INSTRUCTION,                      \
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_WRITE_POLICY_WRITE_BACK                      \
  }

// PPTT cache attributes for unified cache
#define PPTT_UNIFIED_CACHE_ATTR                                                \
  {                                                                            \
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_ALLOCATION_READ_WRITE,                       \
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_CACHE_TYPE_UNIFIED,                          \
    EFI_ACPI_6_4_CACHE_ATTRIBUTES_WRITE_POLICY_WRITE_BACK                      \
  }

/** Helper macro to calculate a unique cache ID

  Macro to calculate a unique 32 bit cache ID. The 32-bit encoding format of the
  cache ID is
  * Bits[31:24]: Unused
  * Bits[23:20]: Package number the cache belongs to
  * Bits[19:12]: Cluster number the cache belongs to (0, if not a cluster cache)
  * Bits[11:4] : Core number the cache belongs to (0, if not a CPU cache)
  * Bits[3:0]  : Cache Type (as defined by RD_PPTT_CACHE_TYPE)

  Note: Cache ID zero is invalid as per ACPI 6.4 specification. Also this
  calculation is not based on any specification.

  @param [in] PackageId Package instance number.
  @param [in] ClusterId Cluster instance number (for Cluster cache, 0 otherwise)
  @param [in] CpuId     CPU instance number (for CPU cache, 0 otherwise).
  @param [in] CacheType Identifier for cache type as defined by
                        RD_PPTT_CACHE_TYPE.
**/
#define RD_PPTT_CACHE_ID(PackageId, ClusterId, CoreId, CacheType)              \
    (                                                                          \
      (((PackageId) & 0xF) << 20) | (((ClusterId) & 0xFF) << 12) |             \
      (((CoreId) & 0xFF) << 4) | ((CacheType) & 0xF)                           \
    )

// EFI_ACPI_6_4_PPTT_STRUCTURE_PROCESSOR
#define EFI_ACPI_6_4_PPTT_STRUCTURE_PROCESSOR_INIT(Length, Flag, Parent,       \
  ACPIProcessorID, NumberOfPrivateResource)                                    \
  {                                                                            \
    EFI_ACPI_6_4_PPTT_TYPE_PROCESSOR,                 /* Type 0 */             \
    Length,                                           /* Length */             \
    {                                                                          \
      EFI_ACPI_RESERVED_BYTE,                                                  \
      EFI_ACPI_RESERVED_BYTE,                                                  \
    },                                                                         \
    Flag,                                             /* Processor flags */    \
    Parent,                                           /* Ref to parent node */ \
    ACPIProcessorID,                                  /* UID, as per MADT */   \
    NumberOfPrivateResource                           /* Resource count */     \
  }

// EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE
#define EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE_INIT(Flag, NextLevelCache, Size,     \
  NoOfSets, Associativity, Attributes, LineSize, CacheId)                      \
  {                                                                            \
    EFI_ACPI_6_4_PPTT_TYPE_CACHE,                     /* Type 1 */             \
    sizeof (EFI_ACPI_6_4_PPTT_STRUCTURE_CACHE),       /* Length */             \
    {                                                                          \
      EFI_ACPI_RESERVED_BYTE,                                                  \
      EFI_ACPI_RESERVED_BYTE,                                                  \
    },                                                                         \
    Flag,                                             /* Cache flags */        \
    NextLevelCache,                                   /* Ref to next level */  \
    Size,                                             /* Size in bytes */      \
    NoOfSets,                                         /* Num of sets */        \
    Associativity,                                    /* Num of ways */        \
    Attributes,                                       /* Cache attributes */   \
    LineSize,                                         /* Line size in bytes */ \
    CacheId                                           /* Cache id */           \
  }

#endif /* __SGI_ACPI_HEADER__ */
