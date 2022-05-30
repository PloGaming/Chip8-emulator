#ifndef CHIP8MEMORY_H
#define CHIP8MEMORY_H

#include <stdint.h>
#include "config.h"

// La RAM del nostro chip8
struct chip8_memory
{
    uint8_t memory[CHIP8_MEMORY_SIZE];
};

// Funzioni per i/o per la RAM del chip8
void chip8_memory_set(struct chip8_memory *mem, int index, unsigned char val);
uint8_t chip8_memory_get(struct chip8_memory *mem, int index);

// Questa funzione leggerà 2 byte e li ritornerà insieme, questo perchè lo spazio di un bytecode è esattamente di 2 byte
uint16_t chip8_memory_get_short(struct chip8_memory *memory, int index);

#endif
