/**
 * Copyright (c) 2023, amarioguy (AppleWOA authors).
 * 
 * Module Name:
 *     DSDT.asl
 * 
 * Abstract:
 *     Differentiated System Description Table. This source file implements the DSDT table
 *     for the base MacBook Pro (Early 2023) platform.
 *     Variants will be handled via SSDTs loaded depending on the platform.
 * 
 * Environment:
 *     UEFI firmware/runtime services.
 * 
 * License:
 *     SPDX-License-Identifier: BSD-2-Clause-Patent OR MIT
 * 
 **/
#include <IndustryStandard/Acpi65.h>

 DefinitionBlock("DSDT.aml", "DSDT", 0x02, "Apple", "J41x", 0x6020) {
    Scope(\_SB) {

        //
        // Cluster Low Power States defined here. Seem to be the same for E-cores/P-cores?
        // Note: there is a state where the cluster can be powered off, unsure how to use so not implemented.
        // If all cores in a cluster are in "deep WFI" mode, the cluster enters "deep WFI" as well.
        //

        Name (CLPI, Package() {
            0, // Version
            0, // Level Index
            1, // Count
            Package() { // Power Gating state for Cluster
            1, // Min residency (uS)
            1, // Wake latency (uS)
            1, // Flags
            1, // Arch Context Flags
            0, //Residency Counter Frequency
            0, // No Parent State
            0x00000000, // Integer Entry method (currently NULL, TODO actually add an entry method)
            ResourceTemplate() { // Null Residency Counter
                Register (SystemMemory, 0, 0, 0, 0)
            },
            ResourceTemplate() { // Null Usage Counter
                Register (SystemMemory, 0, 0, 0, 0)
            },
            "ClusterRetention"
            },
        })

        //
        // Per processor low power states. Currently shallow/deep WFI, suspend to ram not implemented yet.
        // Side note, not gonna be fun having ACPI handle the reconfig engine...
        // TODO: actually implement this.
        //
        Name(PLPI, Package() {
            0, // Version
            0, // Level Index
            2, // Count
            Package() { // WFI for CPU
            1, // Min residency (uS)
            1, // Wake latency (uS)
            1, // Flags
            0, // Arch Context Flags
            0, //Residency Counter Frequency
            0, // No parent state
            ResourceTemplate () {
                // Register Entry method
                Register (SystemMemory,
                0x00,               // Bit Width
                0x00,               // Bit Offset
                0x00,         // Address
                0x00,               // Access Size
                )
            },
            ResourceTemplate() { // Null Residency Counter
                Register (SystemMemory, 0, 0, 0, 0)
            },
            ResourceTemplate() { // Null Usage Counter
                Register (SystemMemory, 0, 0, 0, 0)
            },
            "WFI",
            },
            Package() { // Power Gating state for CPU
            1, // Min residency (uS)
            1, // Wake latency (uS)
            1, // Flags
            1, // Arch Context Flags
            0, //Residency Counter Frequency
            1, // Parent node can be in any state
            ResourceTemplate () {
                // Register Entry method
                Register (SystemMemory,
                0x00,               // Bit Width
                0x00,               // Bit Offset
                0x00000000,         // Address
                0x00,               // Access Size
                )
            },
            ResourceTemplate() { // Null Residency Counter
                Register (SystemMemory, 0, 0, 0, 0)
            },
            ResourceTemplate() { // Null Usage Counter
                Register (SystemMemory, 0, 0, 0, 0)
            },
            "CorePwrDn"
            },
        })

        // Device(PCI0) {
        //     Name (_HID, EISAID ("PNP0A08")) // PCI Express Root Bridge
        //     Name (_CID, EISAID ("PNP0A03")) // Compatible PCI Root Bridge
        //     Name (_SEG, Zero) // PCI Segment Group number
        //     Name (_BBN, Zero) // PCI Base Bus Number
        //     Name (_ADR, Zero)
        //     Name (_UID, "PCI0")
        // }

        Device (XHC1) {
            Name (_HID, "PNP0D15")
            Name (_UID, One)
            Name (_CCA, One)

            Method (_CRS, 0, Serialized) {
                Name (RBUF, ResourceTemplate () {
                    QWordMemory (
                        ResourceConsumer,     // ResourceUsage
                        PosDecode,            // Decode
                        MinFixed,             // IsMinFixed
                        MaxFixed,             // IsMaxFixed
                        NonCacheable,         // Cacheable
                        ReadWrite,            // ReadAndWrite
                        0,                    // AddressGranularity - GRA
                        0xB02280000,          // AddressMinimum - MIN
                        0xB0237FFFF,          // AddressMaximum - MAX
                        0,                    // AddressTranslation - TRA
                        0x100000              // RangeLength - LEN
                        )
                    Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive) {
                        1274
                    }
                })
                Return (RBUF)
            }

            Method (_STA) {
                Return (0xF)
            }
        }

        //
        // All known Apple devices to date have used the Samsung based UART that debuted on the 8900 (or a compatible implementation).
        //
        Device(COM0) {
            Name(_HID, "APPL8900") // naming it APPL8900 since the Samsung based UART was used since the S5L8900
            Name(_UID, Zero)
            Name (_CRS, ResourceTemplate () {
                QWordMemory (
                ResourceProducer,     // ResourceUsage
                PosDecode,            // Decode
                MinFixed,             // IsMinFixed
                MaxFixed,             // IsMaxFixed
                NonCacheable,         // Cacheable
                ReadWrite,            // ReadAndWrite
                0x0000000000000000,   // AddressGranularity - GRA
                0x39b200000,   // AddressMinimum - MIN
                0x39b200fff,   // AddressMaximum - MAX
                0x0000000000000000,   // AddressTranslation - TRA
                0x0000000000001000    // RangeLength - LEN
                )
                Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 1198 }            
            })
            Method (_STA) {
                Return (0xF)
            }
        }
        //
        // Die 0, always present.
        //
        Device(DIE0) {
            Name(_HID, "ACPI0010") // all "processor containers" must have this HID
            Name(_UID, Zero) // unique identifier of the container
            //
            // E-core cluster, present on all variants.
            //
            Device(CLU0) {
                Name(_HID, "ACPI0010") // all "processor containers" must have this HID
                Name(_UID, 0x1) // unique identifier of the container
                // Method (_LPI, 0, NotSerialized) {
                //     return(CLPI)
                // }
                //
                // Bootstrap cluster, E-core 0
                //
                Device(CPU0) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 0)
                    // Method (_LPI, 0, NotSerialized) {
                    // return(PLPI)
                    // }
                    Method (_STA) {
                        Return (0xF)
                    }
                }
                //
                // Bootstrap cluster, E-core 1
                //
                Device(CPU1) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 1)
                    // Method (_LPI, 0, NotSerialized) {
                    // return(PLPI)
                    // }
                    Method (_STA) {
                        Return (0xF)
                    }
                }
                //
                // Bootstrap cluster, E-core 2
                //
                Device(CPU2) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 2)
                    // Method (_LPI, 0, NotSerialized) {
                    // return(PLPI)
                    // }
                    Method (_STA) {
                        Return (0xF)
                    }
                }
                //
                // Bootstrap cluster, E-core 3
                //
                Device(CPU3) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 3)
                    // Method (_LPI, 0, NotSerialized) {
                    // return(PLPI)
                    // }
                    Method (_STA) {
                        Return (0xF)
                    }
                }
            }
            //
            // P-core cluster 1, present on all variants.
            //
            Device(CLU1) {
                Name(_HID, "ACPI0010") // all "processor containers" must have this HID
                Name(_UID, 0x2) // unique identifier of the container
                // Method (_LPI, 0, NotSerialized) {
                //     return(CLPI)
                // }
                Device(CPU4) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 0x4)
                    // Method (_LPI, 0, NotSerialized) {
                    // return(PLPI)
                    // }
                    Method (_STA) {
                        Return (0xF)
                    }
                }
                Device(CPU5) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 0x5)
                    // Method (_LPI, 0, NotSerialized) {
                    // return(PLPI)
                    // }
                    Method (_STA) {
                        Return (0xF)
                    }
                }
                Device(CPU6) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 0x6)
                    // Method (_LPI, 0, NotSerialized) {
                    // return(PLPI)
                    // }
                    Method (_STA) {
                        Return (0xF)
                    }
                }
                Device(CPU7) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 0x7)
                    // Method (_LPI, 0, NotSerialized) {
                    // return(PLPI)
                    // }
                    Method (_STA) {
                        Return (0xF)
                    }
                }
            }
            //
            // P-core cluster 2, present on all variants
            //
            Device(CLU2) {
                Name(_HID, "ACPI0010") // all "processor containers" must have this HID
                Name(_UID, 0x3) // unique identifier of the container
                // Method (_LPI, 0, NotSerialized) {
                //     return(CLPI)
                // }
                Device(CPU8) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 0x8)
                    // Method (_LPI, 0, NotSerialized) {
                    // return(PLPI)
                    // }
                    Method (_STA) {
                        Return (0xF)
                    }
                }
                Device(CPU9) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 0x9)
                    // Method (_LPI, 0, NotSerialized) {
                    // return(PLPI)
                    // }
                    Method (_STA) {
                        Return (0xF)
                    }
                }
                Device(CPUA) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 0xA)
                    // Method (_LPI, 0, NotSerialized) {
                    // return(PLPI)
                    // }
                    Method (_STA) {
                        Return (0xF)
                    }
                }
                Device(CPUB) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 0xB)
                    // Method (_LPI, 0, NotSerialized) {
                    // return(PLPI)
                    // }
                    Method (_STA) {
                        Return (0xF)
                    }
                }
            }
        }

        // //
        // // Die 1, only present on multi die SoCs.
        // // TODO: make this an SSDT, only being placed in DSDT because testing on an T6002.
        // //

        // Device(DIE1) {
        //     Name(_HID, "ACPI0010") // all "processor containers" must have this HID
        //     Name(_UID, 0x4) // unique identifier of the container
        //     //
        //     // E-core cluster, present on all variants.
        //     //
        //     Device(CLU3) {
        //         Name(_HID, "ACPI0010") // all "processor containers" must have this HID
        //         Name(_UID, 0x5) // unique identifier of the container
        //         // Method (_LPI, 0, NotSerialized) {
        //         //     return(CLPI)
        //         // }
        //         //
        //         // Bootstrap cluster, E-core 0
        //         //
        //         Device(CPUA) {
        //             Name(_HID, "ACPI0007")
        //             Name(_UID, 0xA)
        //             // Method (_LPI, 0, NotSerialized) {
        //             // return(PLPI)
        //             // }
        //             Method (_STA) {
        //                 Return (0xF)
        //             }
        //         }
        //         //
        //         // Bootstrap cluster, E-core 1
        //         //
        //         Device(CPUB) {
        //             Name(_HID, "ACPI0007")
        //             Name(_UID, 0xB)
        //             // Method (_LPI, 0, NotSerialized) {
        //             // return(PLPI)
        //             // }
        //             Method (_STA) {
        //                 Return (0xF)
        //             }
        //         }
        //     }
        //     //
        //     // P-core cluster 3, present on all variants.
        //     //
        //     Device(CLU4) {
        //         Name(_HID, "ACPI0010") // all "processor containers" must have this HID
        //         Name(_UID, 0x6) // unique identifier of the container
        //         // Method (_LPI, 0, NotSerialized) {
        //         //     return(CLPI)
        //         // }
        //         Device(CPUC) {
        //             Name(_HID, "ACPI0007")
        //             Name(_UID, 0xC)
        //             // Method (_LPI, 0, NotSerialized) {
        //             // return(PLPI)
        //             // }
        //             Method (_STA) {
        //                 Return (0xF)
        //             }
        //         }
        //         Device(CPUD) {
        //             Name(_HID, "ACPI0007")
        //             Name(_UID, 0xD)
        //             // Method (_LPI, 0, NotSerialized) {
        //             // return(PLPI)
        //             // }
        //             Method (_STA) {
        //                 Return (0xF)
        //             }
        //         }
        //         Device(CPUE) {
        //             Name(_HID, "ACPI0007")
        //             Name(_UID, 0xE)
        //             // Method (_LPI, 0, NotSerialized) {
        //             // return(PLPI)
        //             // }
        //             Method (_STA) {
        //                 Return (0xF)
        //             }
        //         }
        //         Device(CPUF) {
        //             Name(_HID, "ACPI0007")
        //             Name(_UID, 0xF)
        //             // Method (_LPI, 0, NotSerialized) {
        //             // return(PLPI)
        //             // }
        //             Method (_STA) {
        //                 Return (0xF)
        //             }
        //         }
        //     }
        //     //
        //     // P-core cluster 4, present on all variants
        //     //
        //     Device(CLU5) {
        //         Name(_HID, "ACPI0010") // all "processor containers" must have this HID
        //         Name(_UID, 0x7) // unique identifier of the container
        //         // Method (_LPI, 0, NotSerialized) {
        //         //     return(CLPI)
        //         // }
        //         Device(CU16) {
        //             Name(_HID, "ACPI0007")
        //             Name(_UID, 0x10)
        //             // Method (_LPI, 0, NotSerialized) {
        //             // return(PLPI)
        //             // }
        //             Method (_STA) {
        //                 Return (0xF)
        //             }
        //         }
        //         Device(CU17) {
        //             Name(_HID, "ACPI0007")
        //             Name(_UID, 0x11)
        //             // Method (_LPI, 0, NotSerialized) {
        //             // return(PLPI)
        //             // }
        //             Method (_STA) {
        //                 Return (0xF)
        //             }
        //         }
        //         Device(CU18) {
        //             Name(_HID, "ACPI0007")
        //             Name(_UID, 0x12)
        //             // Method (_LPI, 0, NotSerialized) {
        //             // return(PLPI)
        //             // }
        //             Method (_STA) {
        //                 Return (0xF)
        //             }
        //         }
        //         Device(CU19) {
        //             Name(_HID, "ACPI0007")
        //             Name(_UID, 0x13)
        //             // Method (_LPI, 0, NotSerialized) {
        //             // return(PLPI)
        //             // }
        //             Method (_STA) {
        //                 Return (0xF)
        //             }
        //         }
        //     }
        // }

        //
        // PCIe root complex (because just implementing it in the host bridge library is not enough apparently...)
        // Code adapted from QemuSbsaPkg DSDT in mu_tiano_platforms
        //
        // Device(PCI0) {
        //     Name (_HID, EISAID ("PNP0A08")) // PCI Express Root Bridge
        //     Name (_CID, EISAID ("PNP0A03")) // Compatible PCI Root Bridge
        //     Name (_SEG, Zero) // PCI Segment Group number
        //     Name (_BBN, Zero) // PCI Base Bus Number
        //     Name (_ADR, Zero)
        //     Name (_UID, "PCI0")
        //     Name (_CCA, One) // per FDT, Apple PCIe is DMA coherent

        //     Method (_STA) {
        //         Return (0xF)
        //     }
        //     Method (_CBA, 0, NotSerialized) {
        //         return (FixedPcdGet32 (PcdPciExpressBaseAddress))
        //     }

        //     //
        //     // TODO: Add _PRT method
        //     //

        //     //
        //     // Root complex settings/resources
        //     //

        //     Method (_CRS, 0, Serialized) {
        //     Name (RBUF, ResourceTemplate() {
        //         WordBusNumber(
        //             ResourceProducer, //ResourceUsage (whether bus range is consumed or produced)
        //             MinFixed, // IsMinFixed - is lowest bus number fixed?
        //             MaxFixed, // IsMaxFixed - is highest bus number fixed?
        //             PosDecode, //Decode - decode positive or negative?
        //             0, // AddressGranularity
        //             FixedPcdGet32(PcdPciBusMin), //AddressMinimum
        //             FixedPcdGet32(PcdPciBusMax), //AddressMaximum
        //             0, //AddressTranslation
        //             4 //RangeLength - number of buses
        //         )

        //         //
        //         // 32-bit PCIe BARs
        //         //
        //         DWordMemory(
        //             ResourceProducer,
        //             PosDecode,
        //             MinFixed,
        //             MaxFixed,
        //             NonCacheable,
        //             ReadWrite,
        //             0x00000000,
        //             FixedPcdGet32(PcdPciMmio32Base),
        //             FixedPcdGet32(PcdPciMmio32Base) + FixedPcdGet32(PcdPciMmio32Size) - 1,
        //             FixedPcdGet64(PcdPciMmio32Translation),
        //             FixedPcdGet32(PcdPciMmio32Size)
        //             )

        //         //
        //         // 64-bit PCIe BARs
        //         //
        //         QWordMemory(
        //             ResourceProducer,
        //             PosDecode,
        //             MinFixed,
        //             MaxFixed,
        //             Prefetchable,
        //             ReadWrite,
        //             0x00000000,
        //             FixedPcdGet64(PcdPciMmio64Base),
        //             FixedPcdGet64(PcdPciMmio64Base) + FixedPcdGet32(PcdPciMmio64Size) - 1,
        //             FixedPcdGet64(PcdPciMmio64Translation),
        //             FixedPcdGet64(PcdPciMmio64Size)
        //             )
        //         }) //Name(RBUF)

        //         Return (RBUF)
        //     } //Method(_CRS)

        //     Device (RES0)
        //     {
        //         Name (_HID, "PNP0C02" /* PNP Motherboard Resources */)  // _HID: Hardware ID
        //         Name (_CRS, ResourceTemplate ()  // _CRS: Current Resource Settings
        //         {
        //         QWordMemory (ResourceProducer, PosDecode, MinFixed, MaxFixed, NonCacheable, ReadWrite,
        //         0x0000000000000000,                       // Granularity
        //         FixedPcdGet64 (PcdPciExpressBaseAddress), // Range Minimum
        //         FixedPcdGet64 (PcdPciExpressBarLimit),    // Range Maximum
        //         0x0000000000000000,                       // Translation Offset
        //         FixedPcdGet64 (PcdPciExpressBarSize),     // Length
        //         ,, , AddressRangeMemory, TypeStatic)
        //         })
        //         Method (_STA) {
        //         Return (0xF)
        //         }
        //     }

        // }
    }
}

