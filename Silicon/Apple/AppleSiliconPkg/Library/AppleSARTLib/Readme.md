# Apple SART Library (AppleSARTLib)

## About

This library implements support for the SART on Apple platforms. This is a prerequisite for bringing up NVMe on Apple platforms.
(This is because the storage controllers for Apple platforms, ANS2/ANS3, require this to be working)

## Design philosophy

This is being implemented as a driver library so that the ANS driver can simply take a dependency on this library instead of having to implement that code in the ANS driver itself.