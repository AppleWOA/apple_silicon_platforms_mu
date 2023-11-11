# T8122-specific platform support UEFI package

## About

This package provides platform-specific code for all machines that use an M3 (base) SoC,
internally called T8122/H15G/Ibiza. Mainly going to be for SoC-specific ACPI tables.

Note that this package is intentionally being treated as incompatible with T8120 as of the
writing of this document (11/10/2023), due to uncertainty on if T8120 and T8122 have
major, breaking core differences.

## Why does this package break convention with the approach for previous base M-series SoCs?

Most other packages for base M-series SoCs in this repository are designed to be compatible with both the SoC itself
and the phone variant that the M-series SoC is derived from (in this case this'd be H15P/T8120/Crete), but due to core
differences in the process node that Apple has used for both, and differences in things like the AIC, it is not safe to
presume both chips share enough core components to share the SoC specific packages, and so this repository will make
a distinction between these two chips and treat them as incompatible for now.