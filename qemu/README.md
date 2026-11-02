---
title: Notes on QEMU scripts
author: Ross Alexander
date: September 22, 2026
---

# Precis

The build script, in particular, was designed for a UEFI based layout,
and the partition configuration assumed there would be three
partitions and is structured specifically for them.

The script was later extended to support having a filesystem directly
on top of the block device, without any partition.  This is much
simpler since the is a single filesystem.

# Image Partitioning

There are three partition options.

1. No partition, filesystem or LVM directly on block device
2. GPT, needed for UEFI BIOS
3. MS-DOS, works with SeaBIOS

For u-boot the root device needs to be partitioned, either GPT or MS-DOS.

## GPT for UEFI

### EFISYS

UEFI requires a FAT32 partition flagged as `EFISYS`.  It is supposed
to 100M but 32M is possible.  This is the smallest FAT32 possible.  By
default this is the first partition.  While the smallest FAT32
partition is 32M the EDK2 BIOS will not recognise it.  The minimal
for EDK2 is 36M.

The EFISYS partition needs to be flagged with an `esp` flag, for
EFI System Partition.

### boot

If the root partition is BTRFS or EXT4 but not XFS then u-boot can
access it.  For this reason a small `boot` partition with the Linux
kernel and possibly initrd.

The linux kernel cannot mount a root filesystem on Linux MD or LVM
directly.  To do this an `initrd` is required to activate the MD or
LVM first then pivot to the root root filesystem on disk.

### root

If the kernel is mounting the root filesytem directly then it needs to
know where to find the it.  This can be done by passing either a
partition (not filesystem) UUID, or a device.  The device will
normally be sd* or vd*.  This method is not reliable if the system has
multiple drives as device enumeration is not deterministic.



