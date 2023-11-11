# Apple AIC DXE Driver

## About

This DXE driver/library implements the required functionality to enable AIC (both v1 and v2) support in the UEFI environment for Apple platforms.

Please note this driver is a work in progress, and as such bugs are expected to be present.

TODO: Add AICv3 support, seen in T8122 and T8130 SoCs, check what's different between v2 and v3

## How does AIC differ from GIC?

The primary and most critical difference between the two is that the AIC has no conception of distributors or redistributors. All interrupt routing is performed by the controller itself, and most acknowledgement, masking and clearing of IPIs is done via MMIO operations or implementation defined system registers and *not* a "CPU interface" by the definition in the GIC standard. IPIs on AICv2 additionally are primarily FIQs, with implementation defined system registers to acknowledge/clear the IPI.

GICv3 and newer introduce the concept of LPIs, essentially message based edge triggered interrupts. Both revisions of AIC do not differentiate between the ARM definition of LPI and standard interrupts whatsoever.

On a GIC system, clearing or setting an interrupt is done with the same register, while on AIC, you must set the appropriate bits in a corresponding "set" register (to mask the interrupt) or "clear" register (which unmasks the interrupt).

Timer interrupts on standard ARM systems with a GIC are routed through the GIC, while on Apple chips, timer interrupts are hardwired to FIQ.

Performance counters are similarly delivered as FIQs if opted into.

## How do AICv1 and AICv2 differ?

AICv1 is a legacy holdover from earlier Apple A-series chips, and as such relies a bit less on hardware routing heuristics (instead deferring to standard CPU affinity) for interrupts and IPIs, along with only supporting a single CPU die. One notable difference is that AICv1 on M1v1 supports FIQ "fast" IPIs and legacy IRQ IPIs. Seeing as to boot non open source operating systems it is rather important that IPIs are working, if an AICv1 is detected, it will defer to using legacy IPIs.

AICv2 drops all support for legacy IPIs, leaving only the "fast" variants available. Furthermore, AICv2 supports multi-die configurations and relies heavily on hardware routing heuristics to route interrupts to the right place.

## How do I build with AIC support?

For this UEFI package, AIC support is mostly gated by the AIC_BUILD flag in the M1/M1v2/M2.dsc file. This support is experimental and not yet complete or properly tested at this time. The actual code and logic is in both AppleAicDxe and AppleAicLib modules (this driver's design borrows some conventions from ArmGicDxe/ArmGicLib).

It's highly advised you run this EFI under a thin hypervisor providing a vGIC while the AIC support is experimental. On M2 and newer platforms, it's no longer an impediment to getting virtualization working on the platform.

## How do I add the AIC driver to my own implementation?

If you wish to add the AIC driver to your own UEFI implementation (for example if you're developing an Apple chip emulator or something similar), copy the AppleAicDxe folder into your implementation's Driver store, then add the INF file locations to the implementation's DSC and FDF files.

## Design philosophy

This DXE driver/library is mainly designed to service the interrupts needed during EFI (this is most likely just going to be any interrupts from boot-critical devices and the timers). As such enough is implemented to allow these kinds of interrupts to work successfully but not every interrupt will actually be acted upon beyond a simple acknowledge (PMC and Uncore PMC interrupts are among those that are "ack-only").

The overall design is inspired from the ArmGicDxe driver and it shares similar conventions to that driver.

## Current status of AIC Driver

- AICv2 driver finished, AICv1 in progress