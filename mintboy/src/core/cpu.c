#include "cpu.h"
#include "common.h"

#define PROGRAM_COUNTER_DEFAULT 0x100U

typedef enum {
    FLAG_Z = 0x0080U,
    FLAG_N = 0x0040U,
    FLAG_H = 0x0020U,
    FLAG_C = 0x0010U
} AFFlagMasks;


void LR35902_init(LR35902* cpu, Bus* bus) {
    #ifdef MINTBOY_DEBUG
        SAFETY_ASSERT(cpu);
        SAFETY_ASSERT(bus);
    #endif

    cpu->bus = bus;

}


void LR35902_cycle(LR35902* cpu) {
    #ifdef MINTBOY_DEBUG
        SAFETY_ASSERT(cpu);
    #endif

    uint16_t opcode = cpu->bus->read16(cpu->pc);
    MINTBOY_UNUSED(opcode);

}