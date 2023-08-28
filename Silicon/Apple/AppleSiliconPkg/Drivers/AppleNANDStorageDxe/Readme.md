# Apple NAND Storage DXE Driver (AppleNANDStorageDxe)

## About

This DXE driver implements support for the Apple NAND Storage controller (ANS2/ANS3) on Apple silicon platforms.

## Why is this driver necessary?

In Apple devices, the boot drives are not standard NVMe drives and do not have their own controllers (even in desktops like the Mac Studio, despite being modules, they still
do not use a discrete storage controller on the device itself unlike most PC NVMe drives). The internal NVMe interface on these platforms instead connects to a SoC coprocessor called Apple NAND Storage or ANS. This area of the SoC is the actual storage controller that the NVMe drives interface with, and as is the case with a lot of Apple hardware quirks is non-standard, hence the need for the driver. 

## Why this hardware design?

There are several benefits to this design, chiefly it enables Apple NVMe drives and the data at rest on them to be protected via a technology called "Data Protection" due to the ability to work in tandem with other SoC blocks such as the AES engines.

## Dependencies

ANS2/ANS3 require the hardware SARTs to be working, hence AppleSARTLib is a prerequisite for this driver to build.