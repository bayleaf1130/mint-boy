#ifndef MINTBOY_CPU_H
#define MINTBOY_CPU_H

#include "bus.h"

#include <stdint.h>


/* 

Registers 

16bit 	Hi 	Lo 	Name/Function
AF 	A 	- 	Accumulator & Flags
BC 	B 	C 	BC
DE 	D 	E 	DE
HL 	H 	L 	HL
SP 	- 	- 	Stack Pointer
PC 	- 	- 	Program Counter/Pointer

The Flag Register (lower 8bit of AF register)
Bit 	Name 	Set 	Clr 	Expl.
7 	zf 	Z 	NZ 	Zero Flag
6 	n 	- 	- 	Add/Sub-Flag (BCD)
5 	h 	- 	- 	Half Carry Flag (BCD)
4 	cy 	C 	NC 	Carry Flag

*/


// Declare incomplete type
typedef struct {
    uint16_t af;
    uint16_t bc;
    uint16_t de;
    uint16_t hl;
    uint16_t sp;
    uint16_t pc;
    Bus* bus;
} LR35902;

extern void LR35902_init(LR35902* cpu, Bus* bus);
extern void LR35902_cycle(LR35902* cpu);

#endif // MINTBOY_CPU_H
