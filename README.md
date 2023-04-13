Apple Silicon Project Mu UEFI firmware
===================================================

### Terms

GXF - Guarded Execution Feature, essentially a set of lateral exception levels that complement EL1/EL2

### Disclaimer

I probably do not know what I'm doing 100%. It goes without saying, but *please* do not run this EFI implementation in any serious production environment. I assume no responsibility or liability for usage of the software and/or any issues that arise because of it.

This implementation at the moment is wildly incomplete, and many things are due to be filled out/worked on over time.

### Description

This is an implementation of Project Mu for Apple silicon platforms. This is intended to be a firmware that can bootstrap common operating systems that require UEFI and/or ACPI support (Windows being an example). This iteration is based off the WOA Project's SurfaceDuoPkg firmware (https://github.com/WOA-Project/SurfaceDuoPkg) as well as the reference ArmPkg/ArmPlatformPkg implementation.

Regarding interrupt controllers, this implementation can be built with optional support for the platform's AIC if you intend to run this without a thin hypervisor layer to provide a vGIC.
(Note that PSCI will need to be handled in GL2, implying GXF must be enabled, *or* it must be an EFI runtime service.)

### Status

BootMGFW gets as far as looking for the FADT ACPI table, working on implementing ACPI tables now

### Layout (for non Mu submodules)

Platform/ - code specific to a given platform (MacBook Air, MacBook Pro, Mac Studio, Mac Mini, etc.)
Silicon/Apple/[socmarketingname]Pkg - code specific to an SoC
Silicon/Apple/AppleSiliconPkg - code applicable to all platforms (things like general AIC and DART support)

### Contributions

Contributions are always welcome! Feel free to submit a pull request if you wish to contribute (note that this primarily applies to the parts of the repository that are not Project Mu submodules, refer to Microsoft's Project Mu website if you wish to contribute to Mu upstream repositories.)

### License

Unless stated otherwise, code in this project is licensed under the BSD 2 clause license.

### Credits/Acknowledgements

The Asahi Linux project - https://alx.sh
    Hector Martin (marcan) - https://twitter.com/marcan42
    Sven Peter - https://twitter.com/svenpeter42

The WOA Project (LumiaWOA/DuoWOA) - https://github.com/WOA-Project/

Microsoft - Project Mu - https://microsoft.github.io/mu/

Microsoft - Project Mu Reference Platforms Github Repo - https://github.com/microsoft/mu_tiano_platforms

Junyu Long - OnePlus 6T Mu Firmware - https://github.com/longjunyu2/mu_platforms_oneplus6t

Gustave Monce (gus33000) - https://twitter.com/gus33000

imbushuo - https://twitter.com/imbushuo

