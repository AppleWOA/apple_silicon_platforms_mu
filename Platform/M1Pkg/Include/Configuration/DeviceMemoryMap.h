#ifndef _DEVICE_MEMORY_MAP_H_
#define _DEVICE_MEMORY_MAP_H_

#include <Library/ArmLib.h>

#define MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT 128

/* Below flag is used for system memory */
#define SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES                               \
  EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED |        \
      EFI_RESOURCE_ATTRIBUTE_TESTED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE |     \
      EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |                               \
      EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |                         \
      EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE |                            \
      EFI_RESOURCE_ATTRIBUTE_EXECUTION_PROTECTABLE

typedef enum { 
    NoHob, 
    AddMem, 
    AddDev, 
    MaxMem 
} DeviceMemoryAddHob;

#define MEMORY_REGION_NAME_MAX_LENGTH 32

typedef struct {
  CHAR8                        Name[MEMORY_REGION_NAME_MAX_LENGTH];
  EFI_PHYSICAL_ADDRESS         Address;
  UINT64                       Length;
  DeviceMemoryAddHob           HobOption;
  EFI_RESOURCE_TYPE            ResourceType;
  EFI_RESOURCE_ATTRIBUTE_TYPE  ResourceAttribute;
  EFI_MEMORY_TYPE              MemoryType;
  ARM_MEMORY_REGION_ATTRIBUTES ArmAttributes;
} ARM_MEMORY_REGION_DESCRIPTOR_EX, *PARM_MEMORY_REGION_DESCRIPTOR_EX;

#define MEM_RES EFI_RESOURCE_MEMORY_RESERVED
#define MMAP_IO EFI_RESOURCE_MEMORY_MAPPED_IO
#define SYS_MEM EFI_RESOURCE_SYSTEM_MEMORY

#define SYS_MEM_CAP SYSTEM_MEMORY_RESOURCE_ATTR_CAPABILITIES
#define INITIALIZED EFI_RESOURCE_ATTRIBUTE_INITIALIZED
#define WRITE_COMBINEABLE EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE
#define UNCACHEABLE EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE

#define Reserv EfiReservedMemoryType
#define Conv EfiConventionalMemory
#define BsData EfiBootServicesData
#define RtData EfiRuntimeServicesData
#define MmIO EfiMemoryMappedIO

#define NS_DEVICE ARM_MEMORY_REGION_ATTRIBUTE_NONSECURE_DEVICE
#define WRITE_THROUGH ARM_MEMORY_REGION_ATTRIBUTE_WRITE_THROUGH
#define WRITE_THROUGH_XN ARM_MEMORY_REGION_ATTRIBUTE_WRITE_THROUGH
#define WRITE_BACK ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK
#define WRITE_BACK_XN ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK
#define UNCACHED_UNBUFFERED ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED
#define UNCACHED_UNBUFFERED_XN ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED


static ARM_MEMORY_REGION_DESCRIPTOR_EX gDeviceMemoryDescriptorEx[] = {
    /* Name               Address     Length      HobOption        ResourceAttribute    ArmAttributes
                                                          ResourceType          MemoryType */

    /* MMIO */
    /* Based on U-Boot mappings */
    /* TODO: Should I include the nGnRE mappings that aren't defined by U-Boot? */
    /* TODO: adapt this code for something other than 8gb RAM */
    {"MMIO Range 1",      0x200000000, SIZE_2GB, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"MMIO Range 2",      0x380000000, SIZE_1GB, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"MMIO Range 3",      0x500000000, SIZE_1GB, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"MMIO Range 4",      0x680000000, SIZE_512MB, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"PCIe Range 1",      0x6a0000000, SIZE_512MB, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
    {"PCIe Range 2",      0x6c0000000, SIZE_1GB, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},

    /* System DRAM */
    /* TZ0/1/3 is unmapped for now, ditto with the top of system DRAM */ 
    {"m1n1 Reserved",     0x800000000, 0x1EC0000, NoHob, MEM_RES, SYS_MEM_CAP, Reserv, WRITE_BACK_XN},
    {"RAM Partition",     0x801EC0000, 0x2E140000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv, WRITE_BACK_XN},
    {"UEFI FD image",     0x830000000, 0x400000,   AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
    {"RAM Partition (8GB)",0x830400000, 0x1AEC0C000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv, WRITE_BACK_XN},
    // {"TZ1 Region",        0x9df00c000, 0x8000,    AddMem, MEM_RES, UNCACHEABLE, Reserv, WRITE_BACK_XN},
    // {"TZ0 Region",        0x9df014000, 0x1DC8000, AddMem, MEM_RES, UNCACHEABLE, Reserv, WRITE_BACK_XN},
    // {"TZ3 Region",        0x9e6e24000, 0x19000000, AddMem, MEM_RES, UNCACHEABLE, Reserv, WRITE_BACK_XN},

    /* Terminator for MMU */
    {"Terminator", 0, 0, 0, 0, 0, 0, 0}};

#endif