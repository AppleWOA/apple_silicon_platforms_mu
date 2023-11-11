Apple Silicon Project Mu UEFI firmware
===================================================

### Disclaimer

I probably do not know what I'm doing 100%. It goes without saying, but *please* do not run this EFI implementation in any serious production environment. I assume no responsibility or liability for usage of the software and/or any issues that arise because of it.

This implementation at the moment is wildly incomplete, and many things are due to be filled out/worked on over time.

### Description

This is an implementation of Project Mu for Apple silicon platforms. This is intended to be a firmware that can bootstrap common operating systems that require UEFI and/or ACPI support (Windows being an example). This iteration is based off the WOA Project's SurfaceDuoPkg firmware (https://github.com/WOA-Project/SurfaceDuoPkg) as well as the reference ArmPkg/ArmPlatformPkg implementation. 

This spawned out of an effort to boot Windows on Apple chips without any backing from Microsoft or Apple and this remains the primary use case, but right now it is also being expanded to serve as a base UEFI firmware implementation for Apple ARM64 platforms generally.

Regarding interrupt controllers, this implementation can be built with optional support for the platform's AIC if you intend to run this without a vGIC.

This repository is designed with a strong emphasis on code reuse for platforms whenever possible (because many platforms are derivative of a base platform and thus are very similar internally, and even across generations, a lot of SoCs share peripherals or have commonalities), so most actual code will either be made to be as SoC generic as possible within reason, or platforms will generally be grouped in small umbrellas if the internal platform is similar enough. This design philosophy is to avoid redundancy and to reduce copying and pasting code.

### Status

BootMGFW gets as far as preparing to start the Windows kernel, just need to bring up USB so that I have usable debug infrastructure

### Layout (for non Mu submodules)

Platform/ - code specific to a given platform (MacBook Air, MacBook Pro, Mac Studio, Mac Mini, etc.)
Silicon/Apple/[socmarketingname]Pkg - code specific to an SoC, SoC family, or overall platform
Silicon/Apple/AppleSiliconPkg - code applicable to all platforms (things like general AIC and DART support)

### Contributions

Contributions are always welcome! Feel free to submit a pull request if you wish to contribute (note that this primarily applies to the parts of the repository that are not Project Mu submodules, refer to Microsoft's Project Mu website if you wish to contribute to Mu upstream repositories.)

### License

Unless stated otherwise, code in this project is dual licensed under the BSD 2 clause license or the MIT license.

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

