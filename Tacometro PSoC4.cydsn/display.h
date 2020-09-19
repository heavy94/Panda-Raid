#include "project.h"

#define A_MASK(x) ((x & 0b10000000) >> 7)
#define B_MASK(x) ((x & 0b01000000) >> 6)
#define C_MASK(x) ((x & 0b00100000) >> 5)
#define D_MASK(x) ((x & 0b00010000) >> 4)
#define E_MASK(x) ((x & 0b00001000) >> 3)
#define F_MASK(x) ((x & 0b00000100) >> 2)
#define G_MASK(x) ((x & 0b00000010) >> 1)

#define BIT_POS_0(x)    (x & 1)
#define BIT_POS_1(x)    ((x >> 1) & 1)
#define BIT_POS_2(x)    ((x >> 2) & 1)
#define BIT_POS_3(x)    ((x >> 3) & 1)
#define BIT_POS_4(x)    ((x >> 4) & 1)
#define BIT_POS_5(x)    ((x >> 5) & 1)
#define BIT_POS_6(x)    ((x >> 6) & 1)
#define BIT_POS_7(x)    ((x >> 7) & 1)
#define BIT_POS_8(x)    ((x >> 8) & 1)
#define BIT_POS_9(x)    ((x >> 9) & 1)
#define BIT_POS_10(x)   ((x >> 10) & 1)
#define BIT_POS_11(x)   ((x >> 11) & 1)
#define BIT_POS_12(x)   ((x >> 12) & 1)
#define BIT_POS_13(x)   ((x >> 13) & 1)
#define BIT_POS_14(x)   ((x >> 14) & 1)
#define BIT_POS_15(x)   ((x >> 15) & 1)
#define BIT_POS_16(x)   ((x >> 16) & 1)
#define BIT_POS_17(x)   ((x >> 17) & 1)
#define BIT_POS_18(x)   ((x >> 18) & 1)
#define BIT_POS_19(x)   ((x >> 19) & 1)
#define BIT_POS_20(x)   ((x >> 20) & 1)
#define BIT_POS_21(x)   ((x >> 21) & 1)
#define BIT_POS_22(x)   ((x >> 22) & 1)
#define BIT_POS_23(x)   ((x >> 23) & 1)

//Public functions declaration
void display_init();
void display_test();
void display_update(uint32 speed, uint32 heading, uint32 rpm);