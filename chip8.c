#include "instructions.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

uint8_t chip8_fontset[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
uint8_t chip8_font_stride = 5;

/*
    Initializes all registers and the memory region.
*/
void CHIP8_Initialize(Chip8 *chip8) {
    for (int i = 0; i < 4096; i++) chip8->memory[i] = 0;
    for (int i = 0; i < 16; i++) chip8->V[i] = 0;
    for (int i = 0; i < 16; i++) chip8->stack[i] = 0;
    for (int i = 0; i < 64 * 32; i++) chip8->gfx[i] = 0;
    for (int i = 0; i < 16; i++) chip8->key[i] = 0;

    chip8->pc = 0x200;
    chip8->opcode = 0;
    chip8->I = 0;
    chip8->sp = 0;

    chip8->delay_timer = 0;
    chip8->sound_timer = 0;

    // Load fontset
    memcpy(chip8->memory + MEM_FONT_SET, chip8_fontset, 80);
}

/*
    Loads a program into memory at address 0x200.
    program_size expects the size to be given in bytes (i.e. the length of the program array)
*/
void CHIP8_LoadProgram(Chip8 *chip8, uint8_t *program, size_t program_size) {
    memcpy(chip8->memory + MEM_ROM_RAM, program, program_size);
}

/*
    One emulation cycle.
    Executed the current instruction and updates the timers.
    Should be called 60 times per second for proper emulation.
*/
void CHIP8_EmulateCycle(Chip8 *chip8) {
    for (int i = 0; i < 15; i++) {
        // Fetch Opcode
        chip8->opcode = chip8->memory[chip8->pc] << 8 | chip8->memory[chip8->pc + 1];
        // Ececute Opcode
        call_instruction[(chip8->opcode & 0xF000) >> 12](chip8);
    }

    // Update timers
    if (chip8->delay_timer > 0) chip8->delay_timer--;
    if (chip8->sound_timer > 0) {
        chip8->sound_timer--;
        //printf("BEEP!\n");
    }
}

void CHIP8_RegisterDump(Chip8 *chip8) {
    printf("===== Register Contents =====\n");
    for (int i = 0; i < 16; i++) {
        printf("\tV[%x] = 0x%x\n", i, chip8->V[i]);
    }
    printf("\tI = 0x%x\n", chip8->I);
    printf("===== End of Dump =====\n");
}

void CHIP8_MemoryDump(Chip8 *chip8, uint16_t start, uint16_t length) {
    printf("===== Memory Contents from %x to %x =====\n", start, start + length);
    for (uint16_t i = start; i < start + length; i++) {
        printf("\tmem[%x] = 0x%x\n", i, chip8->memory[i]);
    }
    printf("===== End of Dump =====\n");
}
/* ========== Instructions ========== */


void SUB_clear_return(Chip8* chip8) {
    // 2 possible instructions
    switch (chip8->opcode) {
        case 0x00E0:
            for (int i = 0; i < 64 * 32; i++) chip8->gfx[i] = 0;
            break;
        case 0x00EE:
            chip8->pc = chip8->stack[chip8->sp--];
            break;
        default:
            break;
    }
    chip8->pc += 2;
}

/*
    Set program counter to a given address.
*/
void jump(Chip8 *chip8) {
    chip8->pc = chip8->opcode & 0x0FFF;
}

/*
    Store the current program counter on the stack, increase the stack pointer
    and set the program counter to a given address.
*/
void call(Chip8 *chip8) {
    chip8->sp++;
    chip8->stack[chip8->sp] = chip8->pc;
    chip8->pc = chip8->opcode & 0x0FFF;
}

void skip_eq(Chip8 *chip8) {
    if (chip8->V[(chip8->opcode & 0x0F00) >> 8] == (chip8->opcode & 0x00FF)) chip8->pc += 2;
    chip8->pc += 2;
}

void skip_neq(Chip8 *chip8) {
    if (chip8->V[(chip8->opcode & 0x0F00) >> 8] != (chip8->opcode & 0x00FF)) chip8->pc += 2;
    chip8->pc += 2;
}

void skip_eq_reg(Chip8 *chip8) {
    if (chip8->V[(chip8->opcode & 0x0F00) >> 8] == chip8->V[(chip8->opcode & 0x00F0) >> 4]) chip8->pc += 2;
    chip8->pc += 2;
}

void load_reg(Chip8 *chip8) {
    chip8->V[(chip8->opcode & 0x0F00) >> 8] = chip8->opcode & 0x00FF;
    chip8->pc += 2;
}

void add_const(Chip8 *chip8) {
    chip8->V[(chip8->opcode & 0x0F00) >> 8] += chip8->opcode & 0x00FF;
    chip8->pc += 2;
}

void SUB_arithmetic(Chip8* chip8) {
    if ((chip8->opcode & 0x000F) == 0xE) shift_left(chip8); // shift_left is extra
    else call_sub_8[(chip8->opcode & 0x000F)](chip8);
    chip8->pc += 2;
}

void skip_neq_reg(Chip8 *chip8) {
    if (chip8->V[(chip8->opcode & 0x0F00) >> 8] != chip8->V[(chip8->opcode & 0x00F0) >> 4]) chip8->pc += 2;
    chip8->pc += 2;
}

void load_I(Chip8 *chip8) {
    chip8->I = chip8->opcode & 0x0FFF;
    chip8->pc += 2;
}

void jump_offset(Chip8 *chip8) {
    chip8->pc = (chip8->opcode & 0x0FFF) + chip8->V[0];
}

void random_and(Chip8 *chip8) {
    chip8->V[(chip8->opcode & 0x0F00) >> 8] = (rand() % 256) & (chip8->opcode & 0x00FF);
    chip8->pc += 2;
}

void draw(Chip8 *chip8) {
    uint8_t rows = chip8->opcode & 0x000F;
    uint8_t x = chip8->V[(chip8->opcode & 0x0F00) >> 8];
    uint8_t y = chip8->V[(chip8->opcode & 0x00F0) >> 4];
    int flip_flag = 0;
    for (uint8_t yo = 0; yo < rows; yo++) {
        for (uint8_t xo = 0; xo < 8; xo++) {
            // xCoord and yCoord are the final coordinates where the sprite is displayed (including screen wrapping)
            uint8_t xCoord = (x + xo) % WIDTH;
            uint8_t yCoord = (y + yo) % HEIGHT;

            uint8_t pixel = chip8->gfx[yCoord * WIDTH + xCoord];
            uint8_t spritePixel = (chip8->memory[chip8->I + yo] >> (7 - xo)) & 0x1;
            if (pixel && spritePixel) {
                // if both are 1, VF needs to be set to 1
                flip_flag = 1;
            }
            chip8->gfx[yCoord * WIDTH + xCoord] ^= spritePixel;
        }
    }
    chip8->V[15] = flip_flag;
    chip8->pc += 2;
}

void SUB_skip(Chip8 *chip8) {
    uint8_t x = (chip8->opcode & 0x0F00) >> 8;
    switch (chip8->opcode & 0x00FF) {
        case 0x9E: // skip if key pressed
            if (chip8->key[chip8->V[x]]) chip8->pc += 2;
            break;
        case 0xA1: // skip if key not pressed
            if (!chip8->key[chip8->V[x]]) chip8->pc += 2;
            break;
        default:
            break;
    }
    chip8->pc += 2;
}

void SUB_load_add(Chip8 *chip8) {
    uint8_t x = (chip8->opcode & 0x0F00) >> 8; // the value x that is needed for all F-instructions

    uint8_t pressed = 0; // for 0x0A: is a key pressed?
    uint8_t bytes = x > 15 ? 16 : x + 1; // for 0x55 and 0x65

    switch (chip8->opcode & 0x00FF) {
        case 0x07:
            chip8->V[x] = chip8->delay_timer;
            break;
        case 0x0A:
            for (int i = 0; i < 16; i++) {
                if (chip8->key[i]) {
                    pressed = 1;
                    chip8->V[x] = i; // store the pressed key in Vx
                    break;
                }
            }
            if (!pressed) chip8->pc -= 2; // if no key was pressed, wait (i.e. invert the (later applied) PC increase)
            break;
        case 0x15:
            chip8->delay_timer = chip8->V[x];
            break;
        case 0x18:
            chip8->sound_timer = chip8->V[x];
            break;
        case 0x1E:
            chip8->I += chip8->V[x];
            break;
        case 0x29:
        //if (chip8->V[x] > 9) break; // only digits from 0-9
            chip8->I = MEM_FONT_SET + (chip8->V[x] % 10) * chip8_font_stride;
            break;
        case 0x33:
            chip8->memory[chip8->I] = chip8->V[x] / 100; // hundreds digit
            chip8->memory[chip8->I + 1] = (chip8->V[x] % 100) / 10; // tens digit
            chip8->memory[chip8->I + 2] = ((chip8->V[x] % 100) % 10); // ones digit
            break;
        case 0x55:
            // Make sure we don't copy more than we have (registers)
            memcpy(chip8->memory + chip8->I, chip8->V, bytes);
            break;
        case 0x65:
            // Make sure we don't read more than have (registers)
            memcpy(chip8->V, chip8->memory + chip8->I, bytes);
            break;
    }
    chip8->pc += 2;
}

/* ===== SUB 8 (arithmetic) Instructions ===== */
/* The +2 on the program counter is done in the "SUB_arithmetic" function */

void copy(Chip8 *chip8) {
    chip8->V[(chip8->opcode & 0x0F00) >> 8] = chip8->V[(chip8->opcode & 0x00F0) >> 4];
}

void bit_or(Chip8 *chip8) {
    chip8->V[(chip8->opcode & 0x0F00) >> 8] |= chip8->V[(chip8->opcode & 0x00F0) >> 4];
}

void bit_and(Chip8 *chip8) {
    chip8->V[(chip8->opcode & 0x0F00) >> 8] &= chip8->V[(chip8->opcode & 0x00F0) >> 4];
}

void bit_xor(Chip8 *chip8) {
    chip8->V[(chip8->opcode & 0x0F00) >> 8] ^= chip8->V[(chip8->opcode & 0x00F0) >> 4];
}

void add_reg(Chip8 *chip8) {
    uint16_t result = chip8->V[(chip8->opcode & 0x0F00) >> 8] + chip8->V[(chip8->opcode & 0x00F0) >> 4];
    if (result > 255) {
        chip8->V[15] = 1;
    } else {
        chip8->V[15] = 0;
    }
    // save the lower 8 bits in Vx
    chip8->V[(chip8->opcode & 0x0F00) >> 8] = result & 0x00FF;
}

void sub_reg(Chip8 *chip8) {
    if (chip8->V[(chip8->opcode & 0x0F00) >> 8] > chip8->V[(chip8->opcode & 0x00F0) >> 4]) {
        chip8->V[15] = 1;
    } else {
        chip8->V[15] = 0;
    }
    chip8->V[(chip8->opcode & 0x0F00) >> 8] -= chip8->V[(chip8->opcode & 0x00F0) >> 4];
}

void shift_right(Chip8 *chip8) {
    chip8->V[15] = chip8->V[(chip8->opcode & 0x0F00) >> 8] & 0x01; // set the flag if LSB of x is 1
    chip8->V[(chip8->opcode & 0x0F00) >> 8] >>= 1;
}

void sub_reg_flip(Chip8 *chip8) {
    if (chip8->V[(chip8->opcode & 0x00F0) >> 4] > chip8->V[(chip8->opcode & 0x0F00) >> 8]) {
        chip8->V[15] = 1;
    } else {
        chip8->V[15] = 0;
    }
    chip8->V[(chip8->opcode & 0x0F00) >> 8] = chip8->V[(chip8->opcode & 0x00F0) >> 4] - chip8->V[(chip8->opcode & 0x0F00) >> 8];
}

void shift_left(Chip8 *chip8) {
    chip8->V[15] = (chip8->V[(chip8->opcode & 0x0F00) >> 8] & 0x80) >> 7; // set the flag if MSB of x is 1
    chip8->V[(chip8->opcode & 0x0F00) >> 8] <<= 1;
}
