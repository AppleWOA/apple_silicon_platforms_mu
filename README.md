Apple Silicon Project Mu UEFI firmware
===================================================

### Disclaimer

I probably do not know what I'm doing 100%. It goes without saying, but *please* do not run this EFI implementation in any serious production environment. I assume no responsibility or liability for usage of the software and/or any issues that arise because of it.

This implementation at the moment is wildly incomplete, and many things are due to be filled out/worked on over time.

Also, while the Asahi Linux maintainers have unofficially given guidance for the project and provided some very helpful tips, it does need to be stated that this project is *explicitly not* officially part of or affiliated with the Asahi Linux project and is independent of it.

### Description

This is an implementation of Project Mu for Apple silicon platforms. This is intended to be a firmware that can bootstrap common operating systems that require UEFI and/or ACPI support (Windows being an example). This iteration is based off the WOA Project's mu_andromeda_platforms firmware (https://github.com/WOA-Project/mu_andromeda_platforms) as well as the reference ArmPkg/ArmPlatformPkg implementation. 

This spawned out of an effort to boot Windows on Apple chips without any backing from Microsoft or Apple and this remains the primary use case, but right now it is also being expanded to serve as a base UEFI firmware implementation for Apple ARM64 platforms generally.

Regarding interrupt controllers, this implementation can be built with support for the platform's AIC if you intend to run this without a vGIC (the default), or it can be built assuming a vGIC.

This repository is designed with a strong emphasis on code reuse for platforms whenever possible (because many platforms are derivative of a base platform and thus are very similar internally, and even across generations, a lot of SoCs share peripherals or have commonalities), so most actual code will either be made to be as SoC generic as possible within reason, or platforms will generally be grouped in small umbrellas if the internal platform is similar enough. This design philosophy is to avoid redundancy and to reduce copying and pasting code.

### Status

The UEFI implementation can boot off a external mass storage device off a USB-C port (note: not without hacks, this needs to be addressed), and the Windows loader initializes and successfully creates the RAMDisk, but hangs on the handoff between bootloader and kernel.

### Layout (for non Mu submodules)
```
Platform/ - code specific to a given platform (MacBook Air, MacBook Pro, Mac Studio, Mac Mini, etc.)

Silicon/Apple/[base_SoC_identifier]Pkg - code specific to an SoC or SoC family (usually the latter, but can be SoC specific). Often just used to store configuration
or things like ACPI tables and SMBIOS information.

Silicon/Apple/AppleSiliconPkg - code applicable to all platforms (things like general AIC and DART support). This is where the bulk of the code for this repository will
end up going.
```

### Mu submodules used
```
MU_BASECORE - the primary and most fundamental Mu submodule, provides the firmware base and supporting build infrastructure

Common/TIANO - primarily used for EmbeddedPkg, this contains a lot 

Common/MU - adds some 
```
### Contributions

Contributions are always welcome! Feel free to submit a pull request if you wish to contribute (note that this primarily applies to the parts of the repository that are not Project Mu submodules, refer to Microsoft's Project Mu website if you wish to contribute to Mu upstream repositories.)

### License

Unless stated otherwise, code in this project is dual licensed under either the BSD 2 clause license or the MIT license. (Often some components will be licensed as "(MIT OR BSD-2-clause) AND GPL", due to origins from the Asahi Linux project or other open source code that is GPL licensed, and to maintain compliance with the GPL I have to license that specific code as GPL as well.)

### Credits/Acknowledgements
```
The Asahi Linux project - https://alx.sh - most of the base implementations for hardware support in this project originate from the Asahi Linux project, and being adapted to the EFI/Windows paradigm, without the work these guys do, this would be significantly more difficult to work on. Please check this project out, they've been doing excellent work on Apple hardware bringup!
    Hector Martin (marcan) - https://social.treehouse.systems/@marcan - helped answer a lot of fundamental questions about copyright, licensing of code, and "where to get started" on bringup.
    Sven Peter - https://social.treehouse.systems/@sven - helped answer specifics about some of the hardware support bringup and guide me back on track after I ran into issues that seemed intractable.

The WOA Project (LumiaWOA/DuoWOA) - https://github.com/WOA-Project/ - this project's structure is based on some of the work this project is doing, please check this project out, their work on porting Windows to Surface Duo is also great!

Microsoft - Project Mu - https://microsoft.github.io/mu/ - Base firmware platform.

Microsoft - Project Mu Reference Platforms Github Repo - https://github.com/microsoft/mu_tiano_platforms - provided a reference platform for which to base some design decisions around

Junyu Long - OnePlus 6T Mu Firmware - https://github.com/longjunyu2/mu_platforms_oneplus6t - for understanding how some of the configuration files are implemented.

Gustave Monce (gus33000) - https://linktr.ee/gus33000 - helped answer some challenging questions on Windows bringup and what I would need to bring

imbushuo - https://twitter.com/imbushuo - helped answer some confusing questions on EFI specific quirks.

```