#ifndef MINTBOY_MEMORY_H
#define MINTBOY_MEMORY_H

// 64 KB Total
#define ADDRESS_SPACE_SIZE       0xFFFFU

// ROM BANK 00 16KB
#define MEM_START_ROM_BANK_00    0x0000U
#define MEM_SIZE_ROM_BANK_00     0x4000U

// Switchable ROM BANK 01 - NN 16KB
#define MEM_START_ROM_BANK_NN    0x4000U
#define MEM_SIZE_ROM_BANK_NN     0x4000U

// Video RAM 8KB
#define MEM_START_VRAM           0x8000U
#define MEM_SIZE_VRAM            0x2000U

// Switchable Cartridge Bank 8KB
#define MEM_START_EXT_RAM        0xA000U
#define MEM_SIZE_EXT_RAM         0x2000U

// Work RAM 4KB Bank 00
#define MEM_START_WORK_RAM_00    0xC000U
#define MEM_SIZE_WORK_RAM_00     0x1000U

// Work RAM 4KB Bank 01 - NN
#define MEM_START_WORK_RAM_NN    0xD000U
#define MEM_SIZE_WORK_RAM_NN     0x1000U

// Work RAM Echo Mirror 8KB
#define MEM_START_WORK_RAM_ECHO  0xE000U
#define MEM_SIZE_WORK_RAM_ECHO   0x1E00U

// Sprite Attribute Table (OAM) 160 Bytes
#define MEM_START_OAM            0xFE00U
#define MEM_SIZE_OAM             0x00A0U

// Empty Unusable 96 Bytes
#define MEM_START_UNUSABLE       0xFEA0U
#define MEM_SIZE_UNUSABLE        0x0060U

// IO Ports 76 Bytes
#define MEM_START_IO_PORTS       0xFF00U
#define MEM_SIZE_IO_PORTS        0x004CU

// Empty Unusable 2 52 Bytes
#define MEM_START_UNUSABLE_2     0xFF4CU
#define MEM_SIZE_UNUSABLE_2      0x0034U

// Internal RAM 127 Bytes
#define MEM_START_INTERNAL_RAM   0xFF80U
#define MEM_SIZE_INTERNAL_RAM    0x007FU

// Interupt enable register 1 Byte
#define MEM_START_INT_ENABLE_REG 0xFFFFU
#define MEM_SIZE_INT_ENABLE_REG  0x0001U


#endif // MINTBOY_MEMORY_H