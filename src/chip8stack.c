#include "..\include\chip8stack.h"
#include "..\include\chip8.h"
#include <assert.h>

// Funzione interna per controllare che lo stack_pointer non diventi maggiore dello spazio disponibile
static void is_Stack_Bounds(struct chip8 *chip)
{
    assert(chip->registers.stack_pointer < sizeof(chip->stack.stack));
}

// Inserire un elemento nello stack
void push(struct chip8 *chip, uint16_t value)
{
    chip->registers.stack_pointer += 1;
    is_Stack_Bounds(chip);
    chip->stack.stack[chip->registers.stack_pointer] = value;
}

// Rimuovere un elemento dallo stack
uint16_t pop(struct chip8 *chip)
{
    is_Stack_Bounds(chip);
    uint16_t element = chip->stack.stack[chip->registers.stack_pointer];
    chip->registers.stack_pointer -= 1;
    return element;
}
