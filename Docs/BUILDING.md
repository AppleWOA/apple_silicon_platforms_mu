# Building the UEFI implementation

Here are the steps you need to build a version of the UEFI for a given Apple silicon platform (note that this is going to be specific to a device family, so a firmware for one device family won't work for another device family.)

Disclaimer: Given the very incomplete at present nature of this repository, I strongly do not recommend building your own binaries of the UEFI at the moment, but am posting instructions here to be as transparent as I can be with regards to this project's development. Also, these instructions have only been tested for Linux, as I've not had the time to test building on macOS (it should be possible, but I was running into weird issues with stuart.)

Windows build support is planned, but not immediately.

Also, only the DEBUG build has been tested, so RELEASE builds aren't tested and might need changes.

Dependencies:
```
// these are required (this can only be built with clang for now.)
uuid-dev clang llvm gcc-aarch64-linux-gnu lld git-core git

//
// Note: when Rust support is implemented, the Rust suite will also become a required dependency.
//

//
// I also install the following on my system, but these may not be entirely necessary:
//

/*
acpica-tools //we build the ACPI tables from source on build
bison
binutils-aarch64-linux-gnu
g++-aarch64-linux-gnu
mono-devel // this used to be a requirement when the project started.
*/
```


1. Clone the repository (submodules are going to be initialized and gathered by the Project Mu build system, stuart, so whether you pass `--recursive` doesn't matter)

2. Export CLANGPDB_BIN and CLANGPDB_AARCH64_PREFIX environment variables in your current shell session as follows:

```
export CLANGPDB_BIN=/usr/lib/[llvm_dir]/bin/
export CLANGPDB_AARCH64_PREFIX=aarch64-linux-gnu-

//
// Note: if you don't persist these environment variables, these have to be 
// exported every time you start a new terminal session.
//
```

3. Create a Python virtual environment for the device family you wish to build for, and get dependencies as follows. (this is all at the root of the cloned repository.)

```
python -m venv [name_here] //the name can be anything, usually the gitignore file will have the "canonical" name of the venv

source [venv_name]/bin/activate //this switches python context to the virtual environment.

pip install --upgrade -r pip-requirements.txt //this installs all the Python dependencies for stuart to work correctly in the virtual environment

//
// TARGET is optional, defaults to DEBUG build, and TOOL_CHAIN_TAG must 
// be the same throughout the next 3 commands (currently, only CLANGPDB is
// supported)
// platform_pkg_dir refers to the device you wish to build for.
//
stuart_setup -c Platform/[platform_pkg_dir]/PlatformBuild.py TOOL_CHAIN_TAG=CLANGPDB [TARGET={DEBUG, RELEASE}]

stuart_update -c Platform/[platform_pkg_dir]/PlatformBuild.py TOOL_CHAIN_TAG=CLANGPDB [TARGET={DEBUG, RELEASE}]
```

3. Build the UEFI as follows:

```
stuart_build -c Platform/[platform_pkg_dir]/PlatformBuild.py TOOL_CHAIN_TAG=CLANGPDB [TARGET={DEBUG, RELEASE}]

//
// If successful, The output will be in Build/[platform_pkg_dir_prefix]-AARCH64/[TARGET]_CLANGPDB, with the main UEFI
// binary being the FD file in the FV/ subdirectory. (which can be treated as a raw binary file.)
// Symbols for individual parts of the firmware are in the PDB/ subdirectory, with individual EFI binaries scattered in the
// AARCH64/ subdirectory.
//
```

