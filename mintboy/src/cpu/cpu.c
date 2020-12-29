#include "cpu/cpu.h"
#include <assert.h>

int InitCpu(LR35902* cpu) {
    #ifdef MINTBOY_DEBUG
        assert(cpu);
    #endif
    cpu->af.f = 0x60;
    return 0;
}
