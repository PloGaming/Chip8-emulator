#include "..\include\chip8.h"
#include <memory.h>
#include <stdint.h>
#include <string.h>
#include "..\include\config.h"
#include <assert.h>
#include <stdbool.h>
#include <time.h>
#include "SDL2/SDL.h"
#include <stdlib.h>

const char chip8_default_character_set[] = {
    0xf0, 0x90, 0x90, 0x90, 0xf0,
    0x20, 0x60, 0x20, 0x20, 0x70,
    0xf0, 0x10, 0xf0, 0x80, 0xf0,
    0xf0, 0x10, 0xf0, 0x10, 0xf0,
    0x90, 0x90, 0xf0, 0x10, 0x10,
    0xf0, 0x80, 0xf0, 0x10, 0xf0,
    0xf0, 0x80, 0xf0, 0x90, 0xf0,
    0xf0, 0x10, 0x20, 0x40, 0x40,
    0xf0, 0x90, 0xf0, 0x90, 0xf0,
    0xf0, 0x90, 0xf0, 0x10, 0xf0,
    0xf0, 0x90, 0xf0, 0x90, 0x90,
    0xe0, 0x90, 0xe0, 0x90, 0xe0,
    0xf0, 0x80, 0x80, 0x80, 0xf0,
    0xe0, 0x90, 0x90, 0x90, 0xe0,
    0xf0, 0x80, 0xf0, 0x80, 0xf0,
    0xf0, 0x80, 0xf0, 0x80, 0x80

};

static void isLong(size_t size)
{
    assert(size + CHIP8_PROGRAM_LOAD_ADDRESS < CHIP8_MEMORY_SIZE);
}

void chip8_init(struct chip8 *chip)
{
    memset(chip, 0, sizeof(struct chip8));
    memcpy(&chip->memory.memory, chip8_default_character_set, sizeof(chip8_default_character_set));
}

void chip8_load(struct chip8 *chip, const char *buf, size_t size)
{
    isLong(size);
    memcpy(&chip->memory.memory[CHIP8_PROGRAM_LOAD_ADDRESS], buf, size);
    chip->registers.program_counter = CHIP8_PROGRAM_LOAD_ADDRESS;
}

static char chip8_wait_for_key_press(struct chip8 *chip)
{
    SDL_Event event;
    while (SDL_WaitEvent(&event))
    {
        if (event.type != SDL_KEYDOWN)
            continue;

        char c = event.key.keysym.sym;
        char chip8_key = chip8_keyboard_map(&chip->keyboard, c);
        if (chip8_key != -1)
        {
            return chip8_key;
        }
    }

    return -1;
}

static void chip8_exec_extended_eigth(struct chip8 *chip, unsigned short opcode)
{
    unsigned char x = (opcode >> 8) & 0x000f;
    unsigned char y = (opcode >> 4) & 0x000f;
    unsigned char final_four_bits = opcode & 0x000f;
    unsigned short tmp = 0;

    switch (final_four_bits)
    {

    // Salva il valore del registro Vy nel registro Vx
    case 0x00:
        chip->registers.V[x] = chip->registers.V[y];
        break;

    // Esegue il bitwise OR su Vx e Vy e lo salva in Vx
    case 0x01:
        chip->registers.V[x] = chip->registers.V[x] | chip->registers.V[y];
        break;

    // Esegue il bitwise AND su Vx e Vy e lo salva in Vx
    case 0x02:
        chip->registers.V[x] = chip->registers.V[x] & chip->registers.V[y];
        break;

    // Esegue il bitwise XOR su Vx e Vy e lo salva in Vx
    case 0x03:
        chip->registers.V[x] = chip->registers.V[x] ^ chip->registers.V[y];
        break;

    // ADD: I valore di Vx e Vy sono aggiunti + carry flag Vf
    case 0x04:
        tmp = chip->registers.V[x] + chip->registers.V[y];
        chip->registers.V[0x0f] = false;
        if (tmp > 0xff)
        {
            chip->registers.V[0x0f] = true;
        }
        chip->registers.V[x] = tmp;
        break;

    // SUB: Sottrae il valore di Vy a Vx e lo salva in Vx
    case 0x05:
        chip->registers.V[0x0f] = false;
        if (chip->registers.V[x] > chip->registers.V[y])
        {
            chip->registers.V[0x0f] = true;
        }
        chip->registers.V[x] = chip->registers.V[x] - chip->registers.V[y];
        break;

    // SHR: Se il bit meno significativo di Vx è 1 allora Vf è settato a 1 altrimenti 0
    case 0x06:
        chip->registers.V[0x0f] = chip->registers.V[x] & 0x01;
        chip->registers.V[x] /= 2;
        break;

    // SUBN: Se Vy è maggiore di Vx allora Vf è settato a 1 altrimenti 0
    case 0x07:
        chip->registers.V[0x0f] = chip->registers.V[y] > chip->registers.V[x];
        chip->registers.V[x] = chip->registers.V[y] - chip->registers.V[x];
        break;

    // SHL: Se il bit più significativo di Vx è 1 allora Vf è settato a 1 altrimenti 0
    case 0x0E:
        chip->registers.V[0x0f] = chip->registers.V[x] & 0x80;
        chip->registers.V[x] *= 2;
        break;
    }
}

static void chip8_exec_extended_F(struct chip8 *chip, unsigned short opcode)
{
    unsigned char x = (opcode >> 8) & 0x000f;
    switch (opcode & 0x00ff)
    {

    // LD: Il valore del registro DT viene inserito in Vx
    case 0x07:
        chip->registers.V[x] = chip->registers.delay_timer;
        break;

    // LD: Attende che un pulsante venga premuto
    case 0x0A:
        chip->registers.V[x] = chip8_wait_for_key_press(chip);
        break;

    // LD: Dt diventa Vx
    case 0x15:
        chip->registers.delay_timer = chip->registers.V[x];
        break;

    // LD: St diventa Vx
    case 0x18:
        chip->registers.sound_timer = chip->registers.V[x];
        break;

    // ADD: I valori di I e Vx sono aggiunti e i risultati sono salvati in I
    case 0x1E:
        chip->registers.I += chip->registers.V[x];
        break;

    // LD: Imposta la posizione dello sprite per la cifra Vx
    case 0x29:
        chip->registers.I = chip->registers.V[x] * CHIP8_DEFAULT_SPRITE_HEIGTH;
        break;

    // LD: Salva la rappresentazione BCD di Vx rispettivamente in I, I+1, I+2
    case 0x33:
    {
        uint8_t hundreds = chip->registers.V[x] / 100;
        uint8_t tens = chip->registers.V[x] / 10 % 10;
        uint8_t units = chip->registers.V[x] % 10;

        chip8_memory_set(&chip->memory, chip->registers.I, hundreds);
        chip8_memory_set(&chip->memory, chip->registers.I + 1, tens);
        chip8_memory_set(&chip->memory, chip->registers.I + 2, units);
    }
    break;

    // Salva i registri da V0 a Vx a partire da I
    case 0x55:
    {
        for (int i = 0; i <= x; i++)
        {
            chip8_memory_set(&chip->memory, chip->registers.I + i, chip->registers.V[i]);
        }
    }
    break;

    // Legge i registri da V0 a Vx in memoria da I
    case 0x65:
    {
        for (int i = 0; i <= x; i++)
        {
            chip->registers.V[i] = chip8_memory_get(&chip->memory, chip->registers.I + i);
        }
    }
    break;
    }
}

static void chip8_exec_extended(struct chip8 *chip, uint16_t opcode)
{
    uint16_t nnn = opcode & 0x0fff;
    uint8_t kk = opcode & 0x00ff;
    uint8_t x = (opcode >> 8) & 0x000f;
    uint8_t y = (opcode >> 4) & 0x000f;
    uint8_t n = opcode & 0x000f;

    switch (opcode & 0xf000)
    {
    // JP: Salta ad un indirizzo nnn
    case 0x1000:
        chip->registers.program_counter = nnn;
        break;

    // CALL: chiamata ad una sub-routine
    case 0x2000:
        push(chip, chip->registers.program_counter);
        chip->registers.program_counter = nnn;
        break;

    // SE: Salta l'istruzione successiva se Vx == kk
    case 0x3000:
        if (chip->registers.V[x] == kk)
        {
            chip->registers.program_counter += 2;
        }
        break;

    // SNE: Salta l'istruzione successiva se Vx != kk
    case 0x4000:
        if (chip->registers.V[x] != kk)
        {
            chip->registers.program_counter += 2;
        }
        break;

    // SE: Salta l'istruzione se Vx == Vy
    case 0x5000:
        if (chip->registers.V[x] == chip->registers.V[y])
        {
            chip->registers.program_counter += 2;
        }
        break;

    // LD: Inserisce il valore kk nel registro Vx
    case 0x6000:
        chip->registers.V[x] = kk;
        break;

    // ADD: Aggiunge il valore kk al valore del registro Vx e lo salva nel registro Vx
    case 0x7000:
        chip->registers.V[x] += kk;
        break;

    // Operazioni sui registri
    case 0x8000:
        chip8_exec_extended_eigth(chip, opcode);
        break;

    // SNE: Salta l'istruzione successiva se Vx != Vy
    case 0x9000:
        if (chip->registers.V[x] != chip->registers.V[y])
        {
            chip->registers.program_counter += 2;
        }
        break;

    // Annn: Il valore del registro I è settato a nnn
    case 0xA000:
        chip->registers.I = nnn;
        break;

    // Bnnn: Il program counter p settato a nnn + V0
    case 0xB000:
        chip->registers.program_counter = chip->registers.V[0x00] + nnn;
        break;

    // RND: Imposta il valore di Vx ad un valore "casuale"
    case 0xC000:
        srand(clock());
        chip->registers.V[x] = (rand() % 255) & kk;
        break;

    // DRW: Mostra un n-byte sprite in posizione (Vx, Vy) e se vi è una collisione allora Vf = 1
    case 0xD000:
    {
        const char *sprite = (const char *)&chip->memory.memory[chip->registers.I];
        chip->registers.V[0x0f] = chip8_screen_draw_sprite(&chip->screen, chip->registers.V[x], chip->registers.V[y], sprite, n);
    }
    break;

    // Gestione tasti
    case 0xE000:
    {
        switch (opcode & 0x00ff)
        {

        // SKP: Salta l'istruzione successiva se il pulsante Vx viene premuto
        case 0x9e:
            if (chip8_keyboard_is_down(&chip->keyboard, chip->registers.V[x]))
            {
                chip->registers.program_counter += 2;
            }
            break;

        // SKNP: Salta l'istruzione successiva se il pulsante Vx NON viene premuto
        case 0xa1:
            if (!chip8_keyboard_is_down(&chip->keyboard, chip->registers.V[x]))
            {
                chip->registers.program_counter += 2;
            }
            break;
        }
    }
    break;

    // Disegno e grafica
    case 0xF000:
        chip8_exec_extended_F(chip, opcode);
        break;
    }
}

void chip8_exec(struct chip8 *chip, uint16_t opcode)
{
    switch (opcode)
    {

    // CLS: Pulisce lo schermo
    case 0x00E0:
        chip8_screen_clear(&chip->screen);
        break;

    // RET: Ritorna da una sub-routine
    case 0x00EE:
        chip->registers.program_counter = pop(chip);
        break;

    // Inizio dei casi complessi
    default:
        chip8_exec_extended(chip, opcode);
    }
}
