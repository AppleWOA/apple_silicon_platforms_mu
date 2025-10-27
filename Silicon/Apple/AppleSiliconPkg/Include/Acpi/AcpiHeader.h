/**
 * Copyright (c) 2023, amarioguy (AppleWOA authors).
 * 
 * Module Name:
 *     AcpiHeader.h
 * 
 * Abstract:
 *     ACPI header to be used for all ACPI tables.
 * 
 * Environment:
 *     UEFI firmware.
 * 
 * License:
 *     SPDX-License-Identifier: BSD-2-Clause-Patent OR MIT
 * 
*/

#ifndef ACPI_HEADER_H_
#define ACPI_HEADER_H_

#include <IndustryStandard/Acpi.h>

//
// ACPI table information used to initialize tables.
//
#define EFI_ACPI_OEM_ID           {'A','p','p','l','e'}
#define EFI_ACPI_OEM_TABLE_ID     SIGNATURE_64('A','P','P','L','E','E','F','I')
#define EFI_ACPI_OEM_REVISION     FixedPcdGet32 (PcdAcpiDefaultOemRevision)
#define EFI_ACPI_CREATOR_ID       SIGNATURE_32('A','P','P','L')
#define EFI_ACPI_CREATOR_REVISION FixedPcdGet32 (PcdAcpiDefaultCreatorRevision)

// A macro to initialise the common header part of EFI ACPI tables as defined by
// EFI_ACPI_DESCRIPTION_HEADER structure.
#define __ACPI_HEADER(Signature, Type, Revision) {                \
    Signature,                /* UINT32  Signature */       \
    sizeof (Type),            /* UINT32  Length */          \
    Revision,                 /* UINT8   Revision */        \
    0,                        /* UINT8   Checksum */        \
    EFI_ACPI_OEM_ID,          /* UINT8   OemId[6] */        \
    EFI_ACPI_OEM_TABLE_ID,    /* UINT64  OemTableId */      \
    EFI_ACPI_OEM_REVISION,    /* UINT32  OemRevision */     \
    EFI_ACPI_CREATOR_ID,      /* UINT32  CreatorId */       \
    EFI_ACPI_CREATOR_REVISION /* UINT32  CreatorRevision */ \
  }

#define GET_MPIDR(aff3, aff2, aff1, aff0) \
    ((((UINT64)(aff3) & 0xff) << 32) |    \
     (((UINT64)(aff2) & 0xff) << 16) |    \
     (((UINT64)(aff1) & 0xff) <<  8) |    \
     (((UINT64)(aff0) & 0xff) <<  0))

#endif /* ACPI_HEADER_H_ */
