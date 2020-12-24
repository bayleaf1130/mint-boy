#include "cpu/cpu.h"
#include "util/common.h"

int InitCpu(LR35902* cpu) {
    #if DEBUG
        assert(cpu);
    #endif
    cpu->af.f = 0x60;
    return 0;
}