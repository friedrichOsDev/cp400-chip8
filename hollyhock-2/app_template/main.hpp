#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint8_t v[16]; // V0 - VF

    uint8_t * mem; // pointer to mem
    size_t mem_size; // 4096

    uint8_t * display; // pointer to display
    size_t display_size; // 64x32 pixels

    uint16_t stack[16]; 
    uint16_t sp;

    uint16_t i; // index register
    uint16_t pc; // program counter

    uint8_t delay_timer;
} chip8_hardware_t;