#ifndef TRAP_H
#define TRAP_H

#include "types.h"

typedef struct trapframe {
 /*   0 */  uint64 x0;
 /*   8 */  uint64 x1;
 /*  16 */  uint64 x2;
 /*  24 */  uint64 x3;
 /*  32 */  uint64 x4;
 /*  40 */  uint64 x5;
 /*  48 */  uint64 x6;
 /*  56 */  uint64 x7;
 /*  64 */  uint64 x8;
 /*  72 */  uint64 x9;
 /*  80 */  uint64 x10;
 /*  88 */  uint64 x11;
 /*  96 */  uint64 x12;
 /* 104 */  uint64 x13;
 /* 112 */  uint64 x14;
 /* 120 */  uint64 x15;
 /* 128 */  uint64 x16;
 /* 136 */  uint64 x17;
 /* 144 */  uint64 x18;
 /* 152 */  uint64 x19;
 /* 160 */  uint64 x20;
 /* 168 */  uint64 x21;
 /* 176 */  uint64 x22;
 /* 184 */  uint64 x23;
 /* 192 */  uint64 x24;
 /* 200 */  uint64 x25;
 /* 208 */  uint64 x26;
 /* 216 */  uint64 x27;
 /* 224 */  uint64 x28;
 /* 232 */  uint64 x29;
 /* 240 */  uint64 x30;
 /* 248 */  uint64 elr_el1;     /* holds address to return to when taking exception */
 /* 256 */  uint64 spsr_el1;    /* holds PSTATE */
 /* 264 */  uint64 sp_el0;      /* holds EL0 sp */
 /* 272 */  uint64 esr_el1;     /* expection syndrome */
 /* 280 */  uint64 far_el1;     /* faulting address */
} trapframe;

void kerneltrap(struct trapframe* tf);

#endif
