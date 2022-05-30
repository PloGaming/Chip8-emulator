#include "..\include\chip8memory.h"
#include "..\include\config.h"
#include <assert.h>

static void is_memoryIndex(int index)
{
    assert(index < CHIP8_MEMORY_SIZE && index >= 0);
}

void chip8_memory_set(struct chip8_memory *mem, int index, unsigned char val)
{
    is_memoryIndex(index);
    mem->memory[index] = val;
}

uint8_t chip8_memory_get(struct chip8_memory *mem, int index)
{
    is_memoryIndex(index);
    return mem->memory[index];
}

uint16_t chip8_memory_get_short(struct chip8_memory *memory, int index)
{
    uint8_t byte1 = chip8_memory_get(memory, index);
    uint8_t byte2 = chip8_memory_get(memory, index + 1);

    return byte1 << 8 | byte2;
}
