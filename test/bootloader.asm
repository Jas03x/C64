
const U32 BOOTLOADER_SIZE = 512;
extern U64 kernel_entry;

BOOTLOADER_START:
    // read the kernel from the disk
    MOV(R01,   2); // operation = 2 (read)
    MOV(R02,   9); // start at sector 9 since the first 8 sectors are the bootloader
    MOV(R03,   8); // read 8 sectors, which is 512 bytes (the kernel is 512 bytes at the moment)
    MOV(R04, 512); // the kernel will be loaded into the location right after the bootloader

    JMP(kernel_entry);

BOOTLOADER_END:
    PAD(BOOTLOADER_SIZE - (BOOTLOADER_END - BOOTLOADER_START));

