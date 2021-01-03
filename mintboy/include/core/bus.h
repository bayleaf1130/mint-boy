#ifndef MINTBOY_BUS_H
#define MINTBOY_BUS_H

#include <stdint.h>


// Address space is 16bits or 64KB
typedef void (*write_u8) (uint16_t address, uint8_t value);
typedef void (*write_u16) (uint16_t address, uint16_t value);
typedef void (*write_u32) (uint16_t address, uint32_t value);
typedef uint8_t (*read_u8) (uint16_t address);
typedef uint16_t (*read_u16) (uint16_t address);
typedef uint32_t (*read_u32) (uint16_t address);

typedef struct {
    write_u8 write8;
    write_u16 write16;
    write_u32 write32;
    read_u8 read8;
    read_u16 read16;
    read_u32 read32;
    uint8_t* memory;
} Bus;


#endif // MINTBOY_BUS_H