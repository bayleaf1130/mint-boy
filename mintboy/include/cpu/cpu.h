#ifndef MINTBOY_CPU_H
#define MINTBOY_CPU_H

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

typedef struct {
    struct {
        union {
            uint8_t f;
            struct {
                uint8_t padding: 4;
                uint8_t cy: 1;
                uint8_t h: 1;
                uint8_t n: 1;
                uint8_t zf: 1;
            };
        };
        uint8_t a;
    } af;
    uint16_t bc;
    uint16_t de;
    uint16_t hl;
    uint16_t sp;
    uint16_t pc;
} sm83;

#endif