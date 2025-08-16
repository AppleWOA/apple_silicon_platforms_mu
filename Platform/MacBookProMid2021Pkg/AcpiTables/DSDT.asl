/**
 * Created based of a .dts dump from MacBook Pro 14' M1 Pro (j314s)
 * Created by mat484
 * 
 * Module Name:
 *     DSDT.asl
 * 
 * Abstract:
 *     Differentiated System Description Table. This source file implements the DSDT table
 *     for the Apple MacBook Pro (M1 Pro, 2021) platform.
 *     Based on device tree structure from T6000 SoC.
 * 
 * Environment:
 *     UEFI firmware/runtime services.
 * 
 * License:
 *     SPDX-License-Identifier: BSD-2-Clause-Patent OR MIT
 * 
 **/
#include <IndustryStandard/Acpi65.h>

DefinitionBlock("DSDT.aml", "DSDT", 0x02, "Apple", "J314s", 0x6000) {
    Scope(\_SB) {

        //
        // Cluster Low Power States for M1 Pro (T6000)
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
                0x00000000, // Integer Entry method
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
        // Per processor low power states for M1 Pro
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
                    Register (SystemMemory, 0x00, 0x00, 0x00, 0x00)
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
                    Register (SystemMemory, 0x00, 0x00, 0x00000000, 0x00)
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

        //
        // Apple M1 Pro T6000 UART (Samsung S5L8900 compatible)
        // Based on serial@39b200000 from DTS
        //
        Device(COM0) {
            Name(_HID, "APPL8900")
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
                    0x0000039b200000,     // AddressMinimum - MIN (from DTS)
                    0x0000039b200fff,     // AddressMaximum - MAX
                    0x0000000000000000,   // AddressTranslation - TRA
                    0x0000000000001000    // RangeLength - LEN
                )
                Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 1097 } // 0x449 from DTS           
            })
            Method (_STA) {
                Return (0xF)
            }
        }

        //
        // M1 Pro CPU topology - Single die with E-cores and P-cores
        // Based on cpus section from DTS
        //
        Device(DIE0) {
            Name(_HID, "ACPI0010")
            Name(_UID, Zero)
            
            //
            // E-core cluster (Icestorm) - 2 cores
            // Based on cpu@0 and cpu@1 from DTS with compatible = "apple,icestorm"
            //
            Device(CLU0) {
                Name(_HID, "ACPI0010")
                Name(_UID, 0x1)
                
                Device(CPU0) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 0)
                    Method (_STA) {
                        Return (0xF)
                    }
                }
                
                Device(CPU1) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 1)
                    Method (_STA) {
                        Return (0xF)
                    }
                }
            }
            
            //
            // P-core cluster 1 (Firestorm) - 4 cores
            // Based on cpu@10100, cpu@10101, cpu@10102, cpu@10103 from DTS
            //
            Device(CLU1) {
                Name(_HID, "ACPI0010")
                Name(_UID, 0x2)
                
                Device(CPU2) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 0x2)
                    Method (_STA) {
                        Return (0xF)
                    }
                }
                
                Device(CPU3) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 0x3)
                    Method (_STA) {
                        Return (0xF)
                    }
                }
                
                Device(CPU4) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 0x4)
                    Method (_STA) {
                        Return (0xF)
                    }
                }
                
                Device(CPU5) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 0x5)
                    Method (_STA) {
                        Return (0xF)
                    }
                }
            }
            
            //
            // P-core cluster 2 (Firestorm) - 4 cores  
            // Based on cpu@10200, cpu@10201, cpu@10202, cpu@10203 from DTS
            //
            Device(CLU2) {
                Name(_HID, "ACPI0010")
                Name(_UID, 0x3)
                
                Device(CPU6) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 0x6)
                    Method (_STA) {
                        Return (0xF)
                    }
                }
                
                Device(CPU7) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 0x7)
                    Method (_STA) {
                        Return (0xF)
                    }
                }
                
                Device(CPU8) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 0x8)
                    Method (_STA) {
                        Return (0xF)
                    }
                }
                
                Device(CPU9) {
                    Name(_HID, "ACPI0007")
                    Name(_UID, 0x9)
                    Method (_STA) {
                        Return (0xF)
                    }
                }
            }
        }

        //
        // Interrupt Controller - Apple Interrupt Controller (AIC)
        // Based on interrupt-controller@28e100000 from DTS
        //
        Device(INTC) {
            Name(_HID, "APPL0001")
            Name(_UID, Zero)
            Name (_CRS, ResourceTemplate () {
                QWordMemory (
                    ResourceProducer,
                    PosDecode,
                    MinFixed,
                    MaxFixed,
                    NonCacheable,
                    ReadWrite,
                    0x0000000000000000,   // AddressGranularity
                    0x0000028e100000,     // AddressMinimum - core
                    0x0000028e10bfff,     // AddressMaximum 
                    0x0000000000000000,   // AddressTranslation
                    0x000000000000c000    // RangeLength
                )
                QWordMemory (
                    ResourceProducer,
                    PosDecode,
                    MinFixed,
                    MaxFixed,
                    NonCacheable,
                    ReadWrite,
                    0x0000000000000000,   // AddressGranularity
                    0x0000028e10c000,     // AddressMinimum - event
                    0x0000028e10c003,     // AddressMaximum
                    0x0000000000000000,   // AddressTranslation
                    0x0000000000000004    // RangeLength
                )
            })
            Method (_STA) {
                Return (0xF)
            }
        }

        //
        // Power Management Controller
        // Based on power-management@28e580000 from DTS
        //
        Device(PMGR) {
            Name(_HID, "APPL0002")
            Name(_UID, Zero)
            Name (_CRS, ResourceTemplate () {
                QWordMemory (
                    ResourceProducer,
                    PosDecode,
                    MinFixed,
                    MaxFixed,
                    NonCacheable,
                    ReadWrite,
                    0x0000000000000000,   // AddressGranularity
                    0x0000028e580000,     // AddressMinimum
                    0x0000028e58bfff,     // AddressMaximum
                    0x0000000000000000,   // AddressTranslation
                    0x000000000000c000    // RangeLength
                )
            })
            Method (_STA) {
                Return (0xF)
            }
        }

        //
        // I2C Controllers based on DTS i2c@39b040000, etc.
        //
        Device(I2C0) {
            Name(_HID, "APPL0003")
            Name(_UID, 0)
            Name (_CRS, ResourceTemplate () {
                QWordMemory (
                    ResourceProducer,
                    PosDecode,
                    MinFixed,
                    MaxFixed,
                    NonCacheable,
                    ReadWrite,
                    0x0000000000000000,
                    0x0000039b040000,     // From i2c@39b040000
                    0x0000039b043fff,
                    0x0000000000000000,
                    0x0000000000004000
                )
                Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 1119 } // 0x45f
            })
            Method (_STA) {
                Return (0xF)
            }
        }

        Device(I2C1) {
            Name(_HID, "APPL0003")
            Name(_UID, 1)
            Name (_CRS, ResourceTemplate () {
                QWordMemory (
                    ResourceProducer,
                    PosDecode,
                    MinFixed,
                    MaxFixed,
                    NonCacheable,
                    ReadWrite,
                    0x0000000000000000,
                    0x0000039b044000,     // From i2c@39b044000
                    0x0000039b047fff,
                    0x0000000000000000,
                    0x0000000000004000
                )
                Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 1120 } // 0x460
            })
            Method (_STA) {
                Return (0xF)
            }
        }

        //
        // SPI Controllers based on DTS spi@39b104000, etc.
        //
        Device(SPI0) {
            Name(_HID, "APPL0004")
            Name(_UID, 0)
            Name (_CRS, ResourceTemplate () {
                QWordMemory (
                    ResourceProducer,
                    PosDecode,
                    MinFixed,
                    MaxFixed,
                    NonCacheable,
                    ReadWrite,
                    0x0000000000000000,
                    0x0000039b104000,     // From spi@39b104000
                    0x0000039b107fff,
                    0x0000000000000000,
                    0x0000000000004000
                )
                Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 1107 } // 0x453
            })
            Method (_STA) {
                Return (0xF)
            }
        }

        //
        // USB Controllers based on DTS usb@b02280000, etc.
        //
        Device(USB0) {
            Name(_HID, "APPL0005")
            Name(_UID, 0)
            Name (_CRS, ResourceTemplate () {
                QWordMemory (
                    ResourceProducer,
                    PosDecode,
                    MinFixed,
                    MaxFixed,
                    NonCacheable,
                    ReadWrite,
                    0x0000000000000000,
                    0x000000b02280000,    // From usb@b02280000
                    0x000000b0237ffff,
                    0x0000000000000000,
                    0x0000000000100000
                )
                Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 1207 } // 0x4b7
            })
            Method (_STA) {
                Return (0xF)
            }
        }

        Device(USB1) {
            Name(_HID, "APPL0005")
            Name(_UID, 1)
            Name (_CRS, ResourceTemplate () {
                QWordMemory (
                    ResourceProducer,
                    PosDecode,
                    MinFixed,
                    MaxFixed,
                    NonCacheable,
                    ReadWrite,
                    0x0000000000000000,
                    0x0000007002280000,   // From usb@702280000
                    0x000000702237ffff,
                    0x0000000000000000,
                    0x0000000000100000
                )
                Interrupt(ResourceConsumer, Level, ActiveHigh, Exclusive) { 1198 } // 0x4a6
            })
            Method (_STA) {
                Return (0xF)
            }
        }

        //
        // PCIe Root Complex based on DTS pcie@590000000
        //
        Device(PCI0) {
            Name (_HID, EISAID ("PNP0A08"))
            Name (_CID, EISAID ("PNP0A03"))
            Name (_SEG, Zero)
            Name (_BBN, Zero)
            Name (_ADR, Zero)
            Name (_UID, "PCI0")
            Name (_CCA, One) // DMA coherent per Apple PCIe

            Method (_STA) {
                Return (0xF)
            }

            Name (_CRS, ResourceTemplate () {
                WordBusNumber(
                    ResourceProducer,
                    MinFixed,
                    MaxFixed,
                    PosDecode,
                    0,    // AddressGranularity
                    0,    // AddressMinimum - bus 0
                    4,    // AddressMaximum - bus 4 (from DTS bus-range)
                    0,    // AddressTranslation
                    5     // RangeLength
                )

                // 32-bit memory space
                DWordMemory(
                    ResourceProducer,
                    PosDecode,
                    MinFixed,
                    MaxFixed,
                    NonCacheable,
                    ReadWrite,
                    0x00000000,           // AddressGranularity
                    0xc0000000,           // AddressMinimum - from DTS ranges
                    0xffffffff,           // AddressMaximum
                    0x00000000,           // AddressTranslation
                    0x40000000            // RangeLength
                )

                // 64-bit prefetchable memory space
                QWordMemory(
                    ResourceProducer,
                    PosDecode,
                    MinFixed,
                    MaxFixed,
                    Prefetchable,
                    ReadWrite,
                    0x0000000000000000,   // AddressGranularity
                    0x0000000005a0000000, // AddressMinimum - from DTS ranges  
                    0x0000000005bfffffff, // AddressMaximum
                    0x0000000000000000,   // AddressTranslation
                    0x0000000020000000    // RangeLength
                )
            })
        }

        //
        // Display Controller based on DTS dcp@38bc00000
        //
        Device(DCP0) {
            Name(_HID, "APPL0006")
            Name(_UID, Zero)
            Name (_CRS, ResourceTemplate () {
                QWordMemory (
                    ResourceProducer,
                    PosDecode,
                    MinFixed,
                    MaxFixed,
                    NonCacheable,
                    ReadWrite,
                    0x0000000000000000,
                    0x0000038bc00000,     // From dcp@38bc00000
                    0x0000038bc03fff,
                    0x0000000000000000,
                    0x0000000000004000
                )
                // Additional memory regions for display
                QWordMemory (
                    ResourceProducer,
                    PosDecode,
                    MinFixed,
                    MaxFixed,
                    NonCacheable,
                    ReadWrite,
                    0x0000000000000000,
                    0x0000038a000000,     // disp-0 region
                    0x0000038cffffff,
                    0x0000000000000000,
                    0x0000000003000000
                )
            })
            Method (_STA) {
                Return (0xF)
            }
        }

        //
        // Memory regions and address space
        // Based on memory@10000000000 from DTS
        //
        Device(MEM0) {
            Name(_HID, "PNP0C01") // System Board
            Name(_UID, Zero)
            Name (_CRS, ResourceTemplate () {
                QWordMemory (
                    ResourceProducer,
                    PosDecode,
                    MinFixed,
                    MaxFixed,
                    Cacheable,
                    ReadWrite,
                    0x0000000000000000,   // AddressGranularity
                    0x0000010000000000,   // AddressMinimum - from DTS memory
                    0x00000107cd0f7fff,   // AddressMaximum - calculated from DTS reg
                    0x0000000000000000,   // AddressTranslation
                    0x00000007cd0f8000    // RangeLength - from DTS
                )
            })
            Method (_STA) {
                Return (0xF)
            }
        }
    }
}
