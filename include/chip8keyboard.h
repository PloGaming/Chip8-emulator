#ifndef CHIP8KEYBOARD_H
#define CHIP8KEYBOARD_H

#include <stdio.h>
#include <stdbool.h>
#include "..\include\config.h"

struct chip8_keyboard
{
    bool keyboard[CHIP8_TOTAL_KEYS];
    const char *keyboard_map;
};

// La "key" è il tasto REALE della tastiera e ritorna la "key" VIRTUALE
int chip8_keyboard_map(struct chip8_keyboard *keyboard, char key);

void chip8_keyboard_set_map(struct chip8_keyboard *keyboard, const char *map);
// La "key" è il tasto virtuale ovvero quello che si aspetta il chip8 NON quello della nostra tastiera
void chip8_keyboard_down(struct chip8_keyboard *keyboard, int key);
void chip8_keyboard_up(struct chip8_keyboard *keyboard, int key);
bool chip8_keyboard_is_down(struct chip8_keyboard *keyboard, int key);

#endif
