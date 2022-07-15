Apple Silicon (M1/M1v2/M2) Project Mu UEFI firmware
===================================================

### Terms

M1v2 == M1 Pro/Max/Ultra

GXF - Guarded Execution Feature, essentially a set of lateral exception levels that complement EL1/EL2

### Description

This is an implementation of Project Mu for Apple silicon platforms (at the moment for M1/M1v2/M2). This is intended to be a firmware that can bootstrap common operating systems that require UEFI and/or ACPI support (Windows being an example). This iteration is based off the WOA Project's SurfaceDuoPkg firmware (https://github.com/WOA-Project/SurfaceDuoPkg) as well as the reference

Regarding interrupt controllers, this implementation can be built with optional support for the platform's AIC if you intend to run this in EL2.
(Note that PSCI will need to be handled in GL2, implying GXF must be enabled)


### License

Unless stated otherwise, code in this project is licensed under the BSD 2 clause license

### Credits/Acknowledgements

The Asahi Linux project - https://alx.sh

The WOA Project (LumiaWOA/DuoWOA) - https://github.com/WOA-Project/

Microsoft - Project Mu - https://microsoft.github.io/mu/

Microsoft - Project Mu Reference Platforms Github Repo - https://github.com/microsoft/mu_tiano_platforms

Junyu Long - OnePlus 6T Mu Firmware - https://github.com/longjunyu2/mu_platforms_oneplus6t

gus33000 - https://twitter.com/gus33000