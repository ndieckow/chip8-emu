#include <stdint.h>

#define WIDTH   64
#define HEIGHT  32

#define MEM_FONT_SET    0x050
#define MEM_ROM_RAM     0x200

typedef struct Chip8 {
    // stores the current opcode
    uint16_t opcode;
    uint8_t memory[4096];
    // 16 registers
    uint8_t V[16];

    // Index register
    uint16_t I;
    // Program counter
    uint16_t pc;

    // stack and stack pointer
    uint16_t stack[16];
    uint16_t sp;

    // Screen pixels
    uint8_t gfx[64 * 32];

    // Timers
    uint8_t delay_timer;
    uint8_t sound_timer;

    // Keyboard
    uint8_t key[16];
} Chip8;

void CHIP8_Initialize(Chip8 *chip8);
void CHIP8_LoadProgram(Chip8 *chip8, uint8_t *program, size_t program_size);
void CHIP8_EmulateCycle(Chip8 *chip8);
void CHIP8_RegisterDump(Chip8 *chip8);
void CHIP8_MemoryDump(Chip8 *chip8, uint16_t start, uint16_t length);
