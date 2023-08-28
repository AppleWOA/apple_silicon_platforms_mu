# Apple Silicon embedded firmware blobs

## What is this directory for?

During testing, it was found to be necessary to load firmware blobs before the ANS2 DXE driver was available (and therefore, couldn't be loaded from an EFI system partition).

This folder is the storage location for any such firmware blobs that need to be loaded in case there is no EFI system partition available.

It is intentionally empty on the public repository, so as to avoid any potential licensing issues with distributing firmware blobs. The blobs for your specific machine can be extracted from an Asahi Linux installation, or a macOS IPSW.