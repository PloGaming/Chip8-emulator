#ifndef CHIP8STACK_H
#define CHIP8STACK_H

#include <stdint.h>
#include "..\include\config.h"
struct chip8;

struct chip8_stack
{
    uint16_t stack[CHIP8_TOTAL_STACK_VALUES];
};

void push(struct chip8 *chip, uint16_t value);
uint16_t pop(struct chip8 *chip);

#endif
