Apple Silicon (M1/M1v2/M2) Project Mu UEFI firmware
===================================================

### Terms

M1v2 == M1 Pro/Max/Ultra

### Description

This is an implementation of Project Mu for Apple silicon platforms (at the moment for M1/M1v2/M2). This is intended to be a firmware that can bootstrap common operating systems that require UEFI and/or ACPI support (Windows being an example). This iteration is based off the WOA Project's SurfaceDuoPkg firmware (https://github.com/WOA-Project/SurfaceDuoPkg)

Regarding interrupt controllers, this implementation can be built with optional support for the platform's AIC if you intend to run this in EL2.


### Credits/Acknowledgements

The Asahi Linux project - https://alx.sh

The WOA Project (LumiaWOA/DuoWOA) - https://github.com/WOA-Project/

Microsoft - Project Mu - https://microsoft.github.io/mu/