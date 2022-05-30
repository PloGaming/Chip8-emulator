#ifndef CHIP8REGISTERS_H
#define CHIP8REGISTERS_H

#include "..\include\config.h"
#include <stdint.h>

struct chip8_registers
{
    uint8_t V[CHIP8_TOTAL_DATA_REGISTER];
    uint16_t I;
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint16_t program_counter;
    uint8_t stack_pointer;
};

#endif
