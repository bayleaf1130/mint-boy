#include "cpu/cpu.h"

int InitCpu(sm83* cpu) {
    cpu->af.f = 0b11000000;
}