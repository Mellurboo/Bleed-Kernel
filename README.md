
# Bleed Kernel
A 64-bit Operating System in C using the Limine Bootloader for UEFI and BIOS systems

## Roadmap
- [x]  GDT
- [x]  IDT
- [x]  Exception Handler
- [x]  PMM
- [x]  Page Mapping (vaddr -> paddr conversion and some basic virtual memory allocation but its not really nessisary)
- [ ]  TempFS + VFS
- [ ]  initrd
- [ ]  uACPI integration
- [ ]  APIC
- [ ]  LAPIC
- [ ]  Scheduler
- [ ]  Basic Program Spawning (fork)
- [ ]  Context Switching
- [ ]  Userspace Code
- [ ]  Syscalls
- [ ]  Devices (or equivilant depending on what I want to do)
- [ ]  Usermode Libc
- [ ]  Implement more syscalls as we go porting stuff
- [ ]  Basic PS/2 kbd driver for shell interaction on real hardware
- [ ]  Dynamic Linking / CC for OS Software
- [ ]  Meaty VMM (page swapping etc.)
- [ ]  Various drivers (NVMe, USB etc...)
- [ ]  Persistent filesystems (ext2 probably)

## Run Locally

Virtualise Locally using QEMU (BIOS)
```
make run
```

If you want to specifically use UEFI
```
make run-uefi
```

you can only make the iso without running by doing
```
make all
```

and clean it all up with
```
make clean
```



