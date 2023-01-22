# Apple Silicon Shared Dependencies directory

## What is this directory?

This directory is essentially to store dependencies that can be shared among all Apple Silicon devices in this UEFI implementation. This is done so that platforms don't need to copy code over and over again, and instead can share from this directory. (things like AIC support, DARTs, and stuff like that will usually go here.)

## TODO: Move this to the Silicon/Apple directory.